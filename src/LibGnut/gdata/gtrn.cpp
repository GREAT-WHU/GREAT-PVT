/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/
#include "gdata/gtrn.h"

using namespace std;

namespace gnut
{
    t_gtrn::t_gtrn()
        : t_gobj(),
          _channel(255)
    {
        id_type(TRN);
    }

    t_gtrn::t_gtrn(t_spdlog spdlog)
        : t_gobj(spdlog),
          _channel(255)
    {
        id_type(TRN);
    }

    t_gtrn::~t_gtrn()
    {
    }

    void t_gtrn::header(const t_rxnhdr &hdr, string path)
    {
        _gmutex.lock();

        t_header_pair pair = make_pair(path, hdr);
        _headers.push_back(pair);

        _gmutex.unlock();
    }

    t_gtrn::t_header t_gtrn::headers() const
    {
        return _headers;
    }

    t_rxnhdr t_gtrn::header(string path) const
    {
        _gmutex.lock();

        t_rxnhdr rxnhdr;
        for (auto it = _headers.begin(); it != _headers.end(); ++it)
        {
            if (it->first.compare(path) == 0)
            {
                return it->second;
            }
        }

        return rxnhdr;
    }

    void t_gtrn::channel(int chk)
    {
        _gmutex.lock();
        _channel = chk;
        _gmutex.unlock();

        return;
    }

    int t_gtrn::channel() const
    {
        _gmutex.lock();
        int tmp = _channel;
        _gmutex.unlock();

        return tmp;
    }

} // namespace
