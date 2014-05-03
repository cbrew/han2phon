han2phon
========

Read Korean text (legacy EUC encoding) and output a romanized, syllabified form.

Note
----

This code solves a problem that no-one should really have anymore. Hangul syllables have three components (an initial, a medial and a final). There are 19 initials, 21 medials and 28 finals, so there are 11,172 possible syllables, of which less than a quarter actually occur. When the EUC encoding was designed, people wanted to save space, so designed a rather ad hoc encoding for the 3,000 or so that do occur. Since the mapping from syllables to code points is not fully systematic, han2phon consists mainly of tables spelling out the layout of the codepoints.

Unicode entirely solves this problem, assigning a codepoint to each of the 11,172 syllables, and laying out the code points in the form of a 19 by 21 by 28 array. This makes it incredibly straightforward to map back and forth between codepoints and the corresponding (initial, medial, final) triples. No large tables are needed, just array addressing.
Therefore, it is infinitely preferable either to avoid EUC altogether, or, if you must, simply start by transforming EUC to Unicode, then work with Unicode. 

(Nick explained this to Chris, who wrote the note)

Usage
-----

Compile with 

gcc -o han2phon han2phon.c

You need romaca.c in the directory, but it is included 
directly in han2phon, so don't put it on the command
line.

Sample usage:

$ ./han2phon example.doc 

  

  
  
 oe-sin 
 
    


 ni-ka-la-gwa san-sa-tae-lo  ceon-yeo-myeong sa-mang u-lyeo 

  ma-na-gwa  ni-ka-la-gwa   yeon-hab  ni-ka-la-gwa-e-seo ji-nan
dal  il san-sa-tae-ga il-eo-na bu-lag  gae-leul deop-ci-neun ba-lam-e  ceon
yeo-myeong-i sum-jin geos-eu-lo u-lyeo-doen-da-go hyeon-ji gwan-li-deul-i  il balg-hyeoss-da  
 ni-ka-la-gwa su-do ma-na-gwa-e-seo seo-bug-jjog-eu-lo  baeg  ji-jeom
in po-sol-te-ga-si-yi pel-li-si-ta-seu sel-le-don si-jang-eun jung-mi-ji-yeog-e jib-jung
ho-u-leul peo-bu-un heo-li-ke-in mi-ci lo in-hae gi-ban-i yag-hae-jin san-heo
li-yi il-bu-bun-i mu-neo-jyeo nae-li-myeon-seo san-sa-tae-ga bal-saeng-haess-da-go mal-haess-da  
 sel-le-don si-jang-eun  ji-geum-gga-ji gang-e ddeo-iss-neun sa-ce  gu-leul
hwag-in-haess-da  go mal-ha-go geu-leo-na san-sa-tae ji-yeog-yi ju-min-i  ceon  baeg-yeo
myeong-in jeom-eul gam-an-hal ddae  sa-mang-ja su-neun  ceon-myeong-e i-leul su-do iss
da  go mal-haess-da  
 mi-ci lo in-han sa-mang-ja-neun  il-gga-ji  baeg  myeong-eu-lo gong-sig
jib-gye-dwaess-eu-na i-beon san-sa-tae-lo in-hae keu-ge neul-eo-na-ge dwaess-da  
 i-deul sa-mang-ja-leul na-la-byeol-lo bo-myeon on-du-la-seu-ga  baeg  myeong-eu-lo
ga-jang manh-go ni-ka-la-gwa  baeg  myeong  el sal-ba-do-leu  myeong  gwa-te
mal-la  myeong  ko-seu-ta li-ka  myeong  pa-na-ma-wa meg-si-ko gag  myeong
i-myeo i-bagg-e su baeg-myeong-i sil-jong-dwaess-go su man-myeong-yi i-jae-min-i bal-saeng-haess
da   ggeut 


 yeon-hab    
