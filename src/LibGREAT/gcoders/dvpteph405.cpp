/**
 * @file         dvpteph405.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        decode the binary jpleph_de405 file data 
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gdata/gnavde.h"
#include "gcoders/dvpteph405.h"

using namespace std;

namespace great
{
    t_dvpteph405::t_dvpteph405(t_gsetbase *s, string version, int sz)
        : t_gcoder(s, version, sz),
          _version_de(0),
          _days(0.0)
    {

        _flag = 0;
    }

    int t_dvpteph405::decode_head(char *buff, int bufLen, vector<string> &errmsg)
    {
        _mutex.lock();

        if (t_gcoder::_add2buffer(buff, bufLen) == 0)
        {
            _mutex.unlock();
            return 0;
        }
        _mutex.unlock();
        return -1;
    }

    int t_dvpteph405::decode_data(char *buff, int bufLen, int &cnt, vector<string> &errmsg)
    {

        _mutex.lock();
        if (t_gcoder::_add2buffer(buff, bufLen) == 0)
        {
            _mutex.unlock();
            return 0;
        }

        int i = 0;
        int j = 0;
        int iNRecl = 4;
        int iKsize = 2036;
        int irecsz = iNRecl * iKsize;

        try
        {
            string tmp;
            _decode_buffer.toString(tmp);
            while (tmp.length() >= 4 * irecsz)
            {
                stringstream ss(tmp);
                //read data of first part
                if (_flag == 0)
                {
                    char TTL[14][3][6]; 
                    double SS[3];       //start/end time and intvel days
                    vector<string> constname;
                    vector<double> constval;
                    int constnum;
                    double au, emrat;
                    char str[6];
                    double tmpval;

                    if (tmp.length() < 2 * 4 * irecsz)
                    {
                        break;
                    }
                    for (j = 0; j < 3; j++)
                    {
                        for (i = 0; i < 14; i++)
                        {
                            ss.read(TTL[i][j], sizeof(char) * 6);
                        }
                    }
                    for (i = 0; i < 400; i++)
                    {

                        ss.read(str, sizeof(char) * 6);
                        constname.push_back(string(str, 6));
                    }
                    for (i = 0; i < 3; i++)
                    {
                        ss.read((char *)&SS[i], SIZE_DBL);
                    }
                    _beg_de.from_mjd(int(SS[0] - 2400000.5), 0);
                    _end_de.from_mjd(int(SS[1] - 2400000.5), 0);
                    _days = SS[2];

                    ss.read((char *)&constnum, SIZE_INT);
                    ss.read((char *)&au, SIZE_DBL);
                    ss.read((char *)&emrat, SIZE_DBL);
                    for (j = 0; j < 12; j++)
                    {
                        for (i = 0; i < 3; i++)
                        {
                            ss.read((char *)&_ipt[i][j], SIZE_INT);
                        }
                    }
                    ss.read((char *)&_version_de, SIZE_INT);
                    for (i = 0; i < 3; i++)
                    {
                        ss.read((char *)&_ipt[i][12], SIZE_INT);
                    }

                    // second part--value of constants
                    ss.seekg(irecsz * 4, ios::beg);
                    for (i = 0; i < 400; i++)
                    {
                        ss.read((char *)&tmpval, SIZE_DBL);
                        constval.push_back(tmpval);
                    }
                    vector<string>::iterator itname = constname.begin() + constnum;
                    vector<double>::iterator itval = constval.begin() + constnum;
                    constname.erase(itname, constname.end());
                    constval.erase(itval, constval.end());

                    //fill head data
                    map<string, t_gdata *>::iterator it = _data.begin();
                    while (it != _data.end())
                    {
                        if (it->second->id_type() == t_gdata::ALLDE)
                        {
                            dynamic_cast<t_gnavde *>(it->second)->add_head(SS, au, emrat, _ipt, constname, constval);
                        }
                        it++;
                    }
                    t_gcoder::_consume(2 * 4 * irecsz);
                    _flag++;
                }
                // Chebychev coefficient
                else if (_flag > 0)
                {
                    vector<double> coeff;
                    t_gtime tm;
                    double tmp;
                    for (i = 0; i < iKsize / 2; i++)
                    {
                        ss.read((char *)&tmp, SIZE_DBL);
                        coeff.push_back(tmp);
                    }
                    tm = _beg_de + (_flag - 1) * _days;

                    //fill chebychev coefficient by time
                    map<string, t_gdata *>::iterator it = _data.begin();
                    while (it != _data.end())
                    {
                        //fill piece coffecient to data
                        if (it->second->id_type() == t_gdata::ALLDE)
                        {
                            dynamic_cast<t_gnavde *>(it->second)->add_data(_flag - 1, coeff);
                        }
                        it++;
                    }
                    _flag++;
                    t_gcoder::_consume(4 * irecsz);
                }

                _decode_buffer.toString(tmp);
            }
            _mutex.unlock();
            return irecsz;
        }
        catch (...)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "ERROR : t_dvpteph405::decode_data throw exception");
            return -1;
        }
    }

}