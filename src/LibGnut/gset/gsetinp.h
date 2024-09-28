/**
*
* @verbatim
    History

    @endverbatim
*
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)

*
* @file        gsetinp.h
* @brief       implements input setting class
* @author      Jan Dousa
* @version     1.0.0
* @date        2012-10-23
*
*/

#ifndef GSETINP_H
#define GSETINP_H

#define XMLKEY_INP "inputs" ///< The defination of input node in XML

#include "gexport/ExportLibGnut.h"
#include <map>
#include <string>
#include <iostream>
#include <vector>

#include "gutils/gtypeconv.h"
#include "gset/gsetbase.h"
#include "gutils/gtime.h"

using namespace std;
using namespace pugi;

namespace gnut
{

    /// input file order is important here!
    enum class IFMT : int
    {
        ATX_INP,        ///< atx file
        BIAS_INP,       ///< BIAS file
        BIASINEX_INP,   ///< BIAS file
        BLQ_INP,        ///< oceanload file
        DE_INP,         ///< DE405 file
        IFCB_INP,       ///< IFCB file
        LEAPSECOND_INP, ///< leapsecond file
        EOP_INP,        ///< poleut1 file
        RINEXC_INP,     ///< RINEXC file
        RINEXN_INP,     ///< RINEXN file
        RINEXO_INP,     ///< rinexo file
        SP3_INP,        ///< SP3 file
        UPD_INP,        ///< upd file
        UNDEF = -1
    };

    /// The class for input settings
    class LibGnut_LIBRARY_EXPORT t_gsetinp : public virtual t_gsetbase
    {
    public:
        /// constructor
        t_gsetinp();
        /// destructor
        ~t_gsetinp();

        /// settings check
        void check();
        /// settings help
        void help();

        /**
         * @brief change from string to IFMT
         * @param[in] s file format
         * @return IFMT : file format
         */
        static IFMT str2ifmt(const string &s);

        /**
         * @brief change from IFMT to string
         * @param[in] f file format
         * @return string : file format
         */
        static string ifmt2str(const IFMT &f);

        /**
         * @brief get format input size
         * @param[in] fmt file format
         * @return int : format input size
         */
        int input_size(const string &fmt);

        /**@brief check input */
        bool check_input(const string &fmt);
        void check_input(const string &fmt, const string &message);

        /**
         * @brief get format inputs (all in multimap)
         * @return multimap<IFMT, string> : format inputs (all in multimap)
         */
        multimap<IFMT, string> inputs_all();

        /**
         * @brief get format inputs (ordered)
         * @param[in] fmt file format
         * @return vector<string> : format inputs (ordered)
         */
        vector<string> inputs(const string &fmt);
        vector<string> inputs(const IFMT &ifmt);

    protected:
        /**
         * @brief get all the file name of ftm
         * @param[in] ftm file format
         * @return vector<string> : all the file name of ftm
         */
        vector<string> _inputs(const string &fmt);
        vector<string> _inputs(const IFMT &fmt);

        /**
         * @brief get all the ftm in input node
         * @return set<string> : all the ftm in input node
         */
        set<string> _iformats();

    protected:
        set<IFMT> _IFMT_supported; ///< vector of supported IFMTs (app-specific)
        bool _chkNavig;            ///< check navigation data or not
        bool _chkHealth;           ///< check data health or not
        string _corrStream;        ///< error message string

    private:
    };

} // namespace

#endif
