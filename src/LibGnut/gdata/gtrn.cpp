/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/
#include "gdata/gtrn.h"


namespace gnut
{
    t_gtrn::t_gtrn() :
        t_gobj(),
        _channel(255)
    {
        id_type(ID_TYPE::TRN);
    }

    t_gtrn::~t_gtrn()
    {
    }

    void t_gtrn::header(const t_rxnhdr& hdr, std::string path)
    {

        t_header_pair pair = std::make_pair(path, hdr);
        _headers.push_back(pair);

    }

    t_gtrn::t_header t_gtrn::headers() const
    {
        return _headers;
    }

    t_rxnhdr t_gtrn::header(std::string path) const
    {

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
        _channel = chk;

        return;
    }

    int t_gtrn::channel() const
    {
        int tmp = _channel;

        return tmp;
    }

} // namespace gnut
