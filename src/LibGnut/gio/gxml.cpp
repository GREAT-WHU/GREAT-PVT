/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>

#include "gio/grtlog.h"
#include "gio/gxml.h"
#include "gutils/gfileconv.h"

using namespace std;
using namespace pugi;

namespace gnut
{
    t_gxml::t_gxml(string s, bool upper)
    {
        _root = s;
        _logxml = 0;
        _name = "";
        _delimiter = "  "; // only for nodes/elements
        _ucase = true;
    }

    t_gxml::~t_gxml()
    {
    }

    int t_gxml::glog_set(t_spdlog spdlog)
    {
        _logxml = spdlog;

        return 0;
    }

    int t_gxml::read(const string &file)
    {
        _gmutexxml.lock();

        _name = file;

        if (!(_irc = _doc.load_file(_name.c_str())))
        {
            SPDLOG_LOGGER_ERROR(_logxml, "XML-config not read file " + _name + " " + string(_irc.description()));
            _gmutexxml.unlock();
            return -1;
        }

        SPDLOG_LOGGER_INFO(_logxml, "XML-config read from file " + _name);

        _gmutexxml.unlock();
        this->check();

        return 0;
    }

    int t_gxml::read_istream(istream &is)
    {
        _gmutexxml.lock();

        if (!(_irc = _doc.load(is)))
        {
            SPDLOG_LOGGER_ERROR(_logxml, "XML-config not read istream: " + string(_irc.description()));
            _gmutexxml.unlock();
            return -1;
        }

        SPDLOG_LOGGER_INFO(_logxml, "XML-config read from istream");

        _gmutexxml.unlock();
        this->check();
        return 0;
    }

    int t_gxml::write(const string &file)
    {
        _gmutexxml.lock();

        ofstream of;
        string name(file); 
        substitute(name, GFILE_PREFIX, "");

        try
        {
            of.open(name.c_str());
        }
        catch (fstream::failure e)
        {
            cerr << "Warning: Exception opening file " << name << ": " << e.what() << endl;
            _gmutexxml.unlock();
            return -1;
        }

        _doc.save_file(name.c_str(), _delimiter.c_str(), pugi::format_indent);
        of.close();

        if (_logxml)
            SPDLOG_LOGGER_INFO(_logxml, "XML-file saved: " + name);

        _gmutexxml.unlock();
        return 0;
    }

    int t_gxml::write_ostream(ostream &os)
    {
        _gmutexxml.lock();

        _doc.save(os, _delimiter.c_str(), pugi::format_indent);

        _gmutexxml.unlock();
        return 0;
    }

    xml_node t_gxml::_default_node(xml_node &node, const char *n, const char *val, bool reset)
    {
        string s(n);
        if (reset)
            node.remove_child(s.c_str());

        xml_node elem = node.child(s.c_str());

        if (!elem)
            elem = node.append_child(s.c_str());

        if (!elem)
        {
            SPDLOG_LOGGER_WARN(_logxml, "warning - cannot create element " + s);
        }

        if (elem && (strcmp(val, "") || reset))
        {
            elem.append_child(pugi::node_pcdata).set_value(val);
        }

        return elem;
    }

    void t_gxml::_default_attr(xml_node &node, const char *n, const string &val, bool reset)
    {
        if (node.attribute(n).empty())
            node.append_attribute(n);
        if (strlen(node.attribute(n).value()) == 0 || reset)
            node.attribute(n).set_value(val.c_str());

        string s = node.attribute(n).value();
        node.attribute(n).set_value(s.c_str());

    }

    void t_gxml::_default_attr(xml_node &node, const char *n, const bool &val, bool reset)
    {
        if (node.attribute(n).empty())
            node.append_attribute(n);
        if (strlen(node.attribute(n).value()) == 0 || reset)
            node.attribute(n).set_value(val);
    }

    void t_gxml::_default_attr(xml_node &node, const char *n, const int &val, bool reset)
    {
        if (node.attribute(n).empty())
            node.append_attribute(n);
        if (strlen(node.attribute(n).value()) == 0 || reset)
            node.attribute(n).set_value(val);
    }

    void t_gxml::_default_attr(xml_node &node, const char *n, const double &val, bool reset)
    {
        if (node.attribute(n).empty())
            node.append_attribute(n);
        if (strlen(node.attribute(n).value()) == 0 || reset)
            node.attribute(n).set_value(val);
    }

} // namespace
