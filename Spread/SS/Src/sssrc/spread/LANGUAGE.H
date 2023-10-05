/*************
* LANGUAGE.H
**************/

#ifndef LANGUAGE_H
#define LANGUAGE_H

#define LANGUAGE_USA        0                   //- US  version resources
#define LANGUAGE_JPN        10000               //- JPN version resources
#define LANGUAGE_CHN        20000               //- CHN version resources

#if defined(SPREAD_JPN)
#define LANGUAGE_BASE       LANGUAGE_JPN        //- Language base resources
#elif defined(SPREAD_CHN)
#define LANGUAGE_BASE       LANGUAGE_CHN        //- Language base resources
#else
#define LANGUAGE_BASE       LANGUAGE_USA        //- Language base resources
#endif

#endif
