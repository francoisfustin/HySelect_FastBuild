typedef  struct tagMonth
{
   TCHAR szName[4];
   int   nDays[2];
} MONTH;

static MONTH CalMonth[12] =
{
#ifdef SS_V40
   {_T("Jan"), {31, 31}},
   {_T("Feb"), {28, 29}},   // Leap Year
   {_T("Mar"), {31, 31}},
   {_T("Apr"), {30, 30}},
   {_T("May"), {31, 31}},
   {_T("Jun"), {30, 30}},
   {_T("Jul"), {31, 31}},
   {_T("Aug"), {31, 31}},
   {_T("Sep"), {30, 30}},
   {_T("Oct"), {31, 31}},
   {_T("Nov"), {30, 30}},
   {_T("Dec"), {31, 31}},
#else
   {_T("JAN"), {31, 31}},
   {_T("FEB"), {28, 29}},   // Leap Year
   {_T("MAR"), {31, 31}},
   {_T("APR"), {30, 30}},
   {_T("MAY"), {31, 31}},
   {_T("JUN"), {30, 30}},
   {_T("JUL"), {31, 31}},
   {_T("AUG"), {31, 31}},
   {_T("SEP"), {30, 30}},
   {_T("OCT"), {31, 31}},
   {_T("NOV"), {30, 30}},
   {_T("DEC"), {31, 31}},
#endif // SS_V40
};

