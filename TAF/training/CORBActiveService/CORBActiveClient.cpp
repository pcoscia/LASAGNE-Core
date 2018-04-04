#define LTM_CORBACTIVECLIENT_CPP

#include <taf/TAF.h>
#include <taf/NamingContext.h>
#include <taf/IORResolver_T.h>

#include <daf/ShutdownHandler.h>
#include <daf/PropertyManager.h>

#include <ace/Arg_Shifter.h>

// Note Include Generated cpp's
#include "LTMTopicDetailsC.cpp"
#include "CORBActiveServiceC.cpp"



namespace { // Anonymous namespace

    int     debug_arg_(DAF::debug());
    inline int  debug_arg(void)
    {
        return debug_arg_;
    }

    bool    naming_arg_(false);
    inline bool naming_arg(void)
    {
        return naming_arg_;
    }

    ACE_Time_Value  period_arg_(1); // 1Second
    inline const ACE_Time_Value & period_arg(void)
    {
        return period_arg_;
    }

    int parse_args(int &argc, ACE_TCHAR *argv[])
    {
        // NOTE: argc is a reference as Arg_Shifter moves the arguments around.

        DAF::print_args(argc, argv, false); // Before we strip any arguments

        // We have to use ACE_Arg_Shifter here as some of the options are
        // not just a simple switch (ie -ORBInitRef) therefore we strip out
        // our known arguments by doing a cur_arg_strncasecmp == 0

        for (ACE_Arg_Shifter args(argc, argv); args.is_anything_left();) {

            if (args.is_option_next()) { // We only look for options (i.e. begin with '-')

                // Debug could be an optional parameter (i.e. -z5 OR -z 5)
                int debug_remain = args.cur_arg_strncasecmp("-z");

                if (debug_remain >= 0) { // Maybe Optional Parameter

                    debug_arg_ = true; // set our base state

                    if (debug_remain && ::isdigit(*(args.get_current() + 2))) { // Advance past "-z"
                        debug_arg_ = ace_range(1, 10, DAF_OS::atoi(args.get_current() + 2));
                    }

                    args.consume_arg(); // Consume the argument

                    // Look for any parameters - cant leave any behind
                    while (args.is_parameter_next()) {
                        if (::isdigit(*args.get_current())) { // Does it have a digit first
                            debug_arg_ = ace_range(1, 10, DAF_OS::atoi(args.get_current()));
                        }
                        args.consume_arg(); // Consume the parameter
                    }

                    continue;
                }

                // Check for -n (use_naming) parameter
                else if (args.cur_arg_strncasecmp("-n") == 0) {
                    naming_arg_ = true; args.consume_arg(); continue;
                }

                // Check for -p (period) parameter
                else if (args.cur_arg_strncasecmp("-p") == 0) {

                    args.consume_arg(); // Consume the argument

                    // Look for any parameters - cant leave any behind

                    while (args.is_parameter_next()) {
                        if (::isdigit(*args.get_current())) {
                            period_arg_.set(ace_range(1, 10, DAF_OS::atoi(args.get_current())), 0);
                        }
                        args.consume_arg();
                    }

                    continue;
                }
            }

            args.ignore_arg(); // Ignore the argument (leave behind)
        }

        DAF::print_args(argc, argv, false); // After we strip arguments

        return 0;
    }

    // Helper template to resolve Service IOR
    template <typename T>
    typename T::_var_type  resolveCORBAService(const char *name, bool use_naming = true)
    {
        TAF::IORResolverChain_T<T> resolver;
        resolver.addResolver(new TAF::InitialRefResolver_T<T>(name, resolver.getMonitor()));
        if (use_naming)
        {
            try
            {
                resolver.addResolver(new TAF::NamingResolver_T<T>(name, resolver.getMonitor(), TheTAFBaseContext()));
            }
            catch (const CORBA::Exception& ce)
            {
                ACE_ERROR((LM_ERROR, ACE_TEXT("ERROR: CORBActiveClient; ")
                    ACE_TEXT("Caught %C while trying to add NamingResolver for %C\n"), ce._info().c_str(), name));
            }
        }
        return resolver.resolve();
    }

    const DAF::ShutdownHandler shutdownHandler_; // Instantiate a Shutdown (CTL-C) handler

} // Anonymous

int main(int argc, char *argv[])
{
    if (parse_args(argc, argv)) {
        ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("INFO: CORBActiveClient; ")
            ACE_TEXT("Unable to parse arguments successfully\n")), -1);
    }

    try {

        // Set the TAFBaseContext property to match what is provided to the service
        DAF::set_property(TAF_BASECONTEXT, DAF::format_args("DSTO/%H", true, false));

        TAF::ORBManager orb(argc, argv); // Initialize our primary ORB instance

        DAF::print_properties();

        orb.run(3, false); // Run the ORB reactor with 3 threads (non-blocking)

        try {

            ltm::CORBActiveService_var svc_ior(resolveCORBAService<ltm::CORBActiveService>(ltm::CORBActiveService_OID, naming_arg()));
            if (CORBA::is_nil(svc_ior.in()))
            {
                ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("INFO: CORBActiveClient; ")
                    ACE_TEXT("Unable to resolve CORBA service successfully\n")), -1);
            }

            while (!shutdownHandler_.has_shutdown()) {

                const ACE_Time_Value td_time(DAF_OS::gettimeofday());

                const ltm::LTMTopicDetails td = {
                    CORBA::ULongLong(td_time.sec()),
                    CORBA::ULong(td_time.usec()),
                    CORBA::ULong(DAF_OS::thread_ID()),
                    "CORBA Topic Push"
                };

                // Scope the timer
                {
                    DAF::HighResTimer hrt("CORBActiveClient::pushTopic() round trip time"); ACE_UNUSED_ARG(hrt);

                    svc_ior->pushTopic(td); // Make the CORBA call
                }

                DAF_OS::sleep(period_arg()); // Wait at loop End
            }

            return 0;

        } catch (const CORBA::Exception &ex) {
            ex._tao_print_exception("CORBActiveClient: FAILURE: CORBActiveService not contactable");
        }

    } catch (const CORBA::Exception &ex) {
        ex._tao_print_exception("CORBActiveClient: Unable to Initialize the ORB instance");
    } DAF_CATCH_ALL {
    }

    return -1;
}
