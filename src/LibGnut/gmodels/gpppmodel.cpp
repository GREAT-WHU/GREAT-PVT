
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *  
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include "gmodels/gpppmodel.h"

namespace gnut
{
    t_gpppmodel::t_gpppmodel(string site, t_spdlog spdlog, t_gsetbase *settings)
        : t_gsppmodel(spdlog, site, settings),
          _gallobj(0)
    {

        if (_trpModStr == TROPMODEL::EXTERN)
            _tropoModel = make_shared<t_gtropo>();
        _phase = dynamic_cast<t_gsetproc *>(_settings)->phase();
        _grad_mf = dynamic_cast<t_gsetproc *>(_settings)->grad_mf();
        _attitudes = dynamic_cast<t_gsetproc *>(_settings)->attitudes();
    }

    t_gpppmodel::t_gpppmodel()
    {
    }
    t_gpppmodel::~t_gpppmodel()
    {
    }
    void t_gpppmodel::setOBJ(t_gallobj *obj)
    {
        _gallobj = obj;
        if (obj)
        {
            _grec = obj->obj(_site);
        }
    }

} // namespace
