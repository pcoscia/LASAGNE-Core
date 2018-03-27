#define LTM_CORBACTIVEDATASUPPORT_CPP

#include "CORBActiveDataSupport.h"

#if !defined(TAF_USES_OPENDDS)

namespace LTM
{
    LTMTopicDetailsTopicAdapter::LTMTopicDetailsTopicAdapter(const LTMTopicDetailsCORBAType &td)
    {
        this->topicTime_.sec_       = td.topicTime_.sec_;
        this->topicTime_.usec_      = td.topicTime_.usec_;
        this->topicID_              = td.topicID_;

        DDS::String_replace(&this->topicData_, td.topicData_.in());
    }

    LTMTopicDetailsCORBAdapter::LTMTopicDetailsCORBAdapter(const LTMTopicDetailsTopicType &td)
    {
        this->topicTime_.sec_       = td.topicTime_.sec_;
        this->topicTime_.usec_      = td.topicTime_.usec_;
        this->topicID_              = td.topicID_;

        this->topicData_            = CORBA::string_dup(td.topicData_);
    }

} // namespace LTM

# endif
