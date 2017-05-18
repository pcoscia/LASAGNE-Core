/***************************************************************
    Copyright 2016, 2017 Defence Science and Technology Group,
    Department of Defence,
    Australian Government

	This file is part of LASAGNE.

    LASAGNE is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3
    of the License, or (at your option) any later version.

    LASAGNE is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with LASAGNE.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************/
#include "VisitorUtils.h"


#include "taf/Version.h"
#include "tao/Version.h"
#include "be_extern.h"
#include "global_extern.h"
#include "idl_global.h"
#include "utl_string.h"

#include <algorithm>
#include "ace/OS_NS_ctype.h"

namespace TAF
{
namespace IDL
{

int
VisitorUtils::char_convert(int in)
{
    int v = ACE_OS::ace_toupper(in);
    v = ( ACE_OS::ace_isspace(v) ? '_' : v);
    v = ( ACE_OS::ace_ispunct(v) ? '_' : v);
    return v;
}


void
VisitorUtils::header_guard_start(ostream &out, const std::string& name)
{
    out << "#ifndef " << name << std::endl
        << "#define " << name << std::endl << std::endl;
}


void
VisitorUtils::header_guard_end(std::ostream &out, const std::string& name)
{
    out << std::endl << "#endif //" << name << std::endl;
}

void
VisitorUtils::data_support_includes(std::ostream &out, const std::string& filename)
{
    out     << "\n#if defined(TAF_USES_DDSCORBA)\n\n"
            << "\n#include \"" << filename << "C.h\"\n"
            << "#include \"" << filename << "S.h\"\n"
            << "\n#elif defined(TAF_USES_NDDS)\n\n"
            << "#include \"" << filename << ".h\"\n"
            << "\n#elif defined(TAF_USES_COREDX)\n\n"
            << "#include \"" << filename << ".hh\"\n"
            << "\n#elif defined(TAF_USES_OPENSPLICE)\n\n"
            << "#include \"ccpp_" << filename << ".h\"\n"
            << "\n#endif\n\n";

    out     << "#if defined(TAF_USES_OPENDDS)\n\n"
            << "#include \"" << filename << "TypeSupportC.h\"\n"
            << "#include \"" << filename << "TypeSupportS.h\"\n"
            << "#include \"" << filename << "TypeSupportImpl.h\"\n"
            << "\n#elif defined(TAF_USES_NDDS)\n\n"
            << "#include \"" << filename << "Support.h\"\n"
            << "#include \"" << filename << "Plugin.h\"\n"
            << "\n#elif defined(TAF_USES_COREDX)\n\n"
            << "#include \"" << filename << "TypeSupport.hh\"\n"
            << "#include \"" << filename << "DataReader.hh\"\n"
            << "#include \"" << filename << "DataWriter.hh\"\n"
            << "\n#elif defined(TAF_USES_OPENSPLICE)\n\n"
            << "\n#if defined(TAF_USES_DDSCORBA)\n\n"
            << "#include \"" << filename << "DcpsC.h\"\n"
            << "\n#else\n\n"
            << "#include \"" << filename << "Dcps.h\"\n"
            << "\n#endif\n"
            << "#include \"" << filename << "Dcps_Impl.h\"\n"
            << "\n#else\n"
            << "#error ERROR : You have not set TAF DDS flags for your build\n"
            << "\n#endif\n\n";

}

std::string
VisitorUtils::create_header_id(const std::string &name, bool unique)
{
    std::string id = name;
    // Need to make it all caps. character transform from . -> _
    // Unique... UUID ?
    if (unique)
    {
        id += "_" + be_global->date_string();
    }

    std::string up = id;

    transform(id.begin(), id.end(),up.begin(), char_convert);

    return up;
}

void
VisitorUtils::generation_comment(ostream &out, const std::string& generator_name)
{
    out << std::endl
        << "/******************************************************" << std::endl
        << "* Code generated by LASAGNE IDL Compiler" << std::endl
        << "* LASAGNE BE Version : " << TAF_VERSION << std::endl
        << "* TAO_IDL FE Version : " << TAO_VERSION << std::endl
        << "* LASAGNE Build Time : " << __DATE__ << std::endl
        << "* Generation Visitor : " << generator_name << std::endl
        << "* Creation Date      : " << be_global->date_string() << std::endl
        << "* Input File         : " << idl_global->stripped_filename()->get_string() << std::endl
        << "******************************************************/" << std::endl << std::endl;
}

void
VisitorUtils::taf_dds_headers(ostream &out)
{
    out << std::endl << "#include \"dds/DDSPubSub.h\"" << std::endl << std::endl;
    //out << std::endl << "#include \"dds/DataSupportMacros.h\"" << std::endl << std::endl;
}

}//namesapce IDL
}//namespace TAF
