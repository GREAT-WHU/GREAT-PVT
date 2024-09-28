/**
 * @file         gsetamb.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        control set from XML
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef GSETAMB_H
#define GSETAMB_H

#define XMLKEY_AMBIGUITY "ambiguity"

#include "gexport/ExportLibGREAT.h"
#include "gset/gsetbase.h"

using namespace std;
using namespace gnut;
namespace great
{
    /** @brief class enum of UPD type. */
    enum class UPDTYPE
    {
        WL,         ///< wide lane.
        EWL,        ///< extra wide lane.
        EWL24,      ///< extra wide lane.(2/4 frequency)
        EWL25,      ///< extra wide lane.(2/5 frequency)
        NL,         ///< narrow lane.
        IFCB,       ///< inter frequency clock bias.
        NONE        ///< none.
    };

    /**
    * @brief change UPDTYPE to string.
    * @param[in]    mode      type of UPD
    * @return       string    type of UPD in string form 
    */
    LibGREAT_LIBRARY_EXPORT string updmode2str(UPDTYPE mode);

    /**
    * @brief change string to UPDTYPE.
    * @param[in]    mode      type of UPD in string form
    * @return       UPDTYPE   type of UPD
    */
    LibGREAT_LIBRARY_EXPORT UPDTYPE str2updmode(string mode);

    /** @brief enum of FIX mode. */
    enum class FIX_MODE
    {
        NO,     ///< float.
        SEARCH, ///< ambiguity search.
    };

    /** @brief enum of UPD mode. */
    enum class UPD_MODE
    {
        UPD    ///< wl upd + nl upd.
    };

    /**
    * @brief        class for set ambiguity fixed xml
    */
    class LibGREAT_LIBRARY_EXPORT t_gsetamb : public virtual t_gsetbase
    {
    public:
        /** @brief default constructor. */
        t_gsetamb();

        /** @brief default destructor. */
        virtual ~t_gsetamb();

        /**
        * @brief settings check.
        */
        void check();

        /**
        * @brief settings help.
        */
        void help();

        /**
        * @brief  get ambiguity fixing mode .
        * @return    FIX_MODE    ambiguity fixing mode
        */
        FIX_MODE fix_mode();

        /**
        * @brief  get upd mode .
        * @return    UPD_MODE    upd mode
        */
        UPD_MODE upd_mode();

        /**
        * @brief  get lambda ratio .
        * @return    double    lambda ratio
        */
        double lambda_ratio();

        /**
        * @brief  get bootstrapping rate.
        * @return    double    bootstrapping rate
        */
        double bootstrapping();

        /**
        * @brief  get minimum common time of two observation arc.
        * @return    double    minimum common time of two observation arc
        */
        double min_common_time();

        /**
        * @brief   get ambiguity decision.
        * @return    map<string, double>    ambiguity decision
        */
        map<string, double> get_amb_decision(string str);

        /**
        * @brief   whether take partial ambiguity fixed mode.
        * @return
            @retval true    take partial ambiguity fixed mode
            @retval false    do not take partial ambiguity fixed mode
        */
        bool part_ambfix();

        /**
        * @brief   value's size which take partial ambiguity fixed mode.
        * @return    int    value's size which take partial ambiguity fixed mode
        */
        int part_ambfix_num();

        /**
        * @brief change string to FIX_MODE.
        * @param[in]  str   fix mode in string form
        * @return    FIX_MODE    type of UPD
        */
        FIX_MODE str2fixmode(string str);
        string fixmode2str(FIX_MODE mode);

        /**
        * @brief change string to UPD mode.
        * @param[in]  str   upd mode in string form
        * @return    UPD_MODE    UPD mode
        */
        UPD_MODE str2upd_mode(string str);

        /**
        * @brief   whether set reference satellite.
        * @return
            @retval true    set reference satellite
            @retval false    do not set reference satellite
        */
        bool isSetRefSat();

        /**
        * @brief get full fix num
        * @return    int full fix num
        */
        int full_fix_num();

    protected:
        map<string, map<string, double>> _default_decision = {
            {"EWL", {{"maxdev", 0.07}, {"maxsig", 0.10}, {"alpha", 1000}}},
            {"WL", {{"maxdev", 0.25}, {"maxsig", 0.10}, {"alpha", 1000}}},
            {"NL", {{"maxdev", 0.25}, {"maxsig", 0.10}, {"alpha", 1000}}}}; ///< default ambiguity decision

    private:
    };

}
#endif
