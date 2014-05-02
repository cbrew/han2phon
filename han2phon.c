#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define BUF 65535

/***********************************************************************
 *   han2rom  -- Korean hangul romanizer for EUC_KR (KS X 1001:1992)   *
 *								       *
 *         Romanizes according to the ugly but machine-friendly	       *
 *         ISO TR 11941:1996 (E) transliteration scheme		       *
 *								       *
 *   Written by Nick Cipollone, Nov/4/2000			       *
 *   Adapted  Chris Brew, Oct/17/2001,Feb/7/2002                       *
 *                                                                     *
 *                                                                     *
 ***********************************************************************/

/* The matrix of hangul syllable romanizations: */

#include "romaca.c"

/* The single hangul characters are off in row 4 of the KS X 1001:1992 table,
   while the hangul syllables are in rows 16-40.  */

char* single_hangul[94] = 
    { "k", "kk", "ks", "n", "nc", "nh", "t", "tt", "r", "rk", "rm", "rp", "rs", "rth", "rph", "rh", "m", "p", "pp", "ps", "s", "ss", "ng", "c", "cc", "ch", "kh", "th", "ph", "h", "a", "ae", "ya", "yae", "eo", "e", "yeo", "ye", "o", "wa", "wae", "oe", "yo", "u", "weo", "we", "wi", "yu", "eu", "yi", "i", " ", "nn", "nt", "ns", "nm", "rks", "rt", "rps", "rm", "rh", "mp", "ms", "mm", "mng", "pk", "pt", "psk", "pst", "pc", "pth", "png", "ppng", "sk", "sn", "st", "sp", "sc", "m", "ngng", "ng", "ngs", "ngm", "phng", "hh", "h", "yoya", "yoyae", "yoe", "yuyeo", "yuye", "yui", "eu", "yi" };


/*
 * A token is a structure that represents the occurrence
 * of a term in a text. It contains
 * -- the text of the term, not necessarily NULL terminated
 * -- start and end offsets into the source text, provided
 *    to allow tokens to be re-associated with their source
 *    for KWIC displays and similar
 * -- a field for type information about the token. Typically
 *    part-of-speech or similar.
 * 
 */


/*
 * In the basic tokenizer token.type takes the following
 * values
 */

typedef enum tokenizer_type_indices {kHangul,
				     kRoman,
				     kWhitespace,
				     kPunct,
				     kMarkup,
				     kNoMoreTypes} tokindex;

typedef const char * Atom;

typedef struct token {
  const unsigned char * text;
  size_t size;
  tokindex type;
  int hyphenate;  /* false if just next to whitespace */
} token,*TOKEN;

typedef struct mapping  {
  const char * nick;
  const char * appendix_n;
} mapping;
 
 

/*
 * Modification by Chris: use knowledge of possible vowels to
 * analyse segment strings. Following is a NULL terminated list
 * of Roman representations of vowels. It matters that the ones
 * that have two letters in the orthography come first, since
 * we use the order of the list to resolve conflicts. 
 *
 * 
 */

static int nospace = 0;

static const mapping osu_korean_vowels[] = {
  {"eu","eu"},
  {"eo","eo"},
  {"ae","ae"},  /* not in Kyuchul's dialect */
  {"oe","oe"}, /* not in the phone set, but is in appendix N */
  {"u","u"},
  {"e","e"},
  {"i","i"},
  {"o","o"},
  {"a","a"},
  {NULL,NULL}};


static const mapping osu_korean_consonants[] = {
  {"ph","p"},
  {"pp","bb"},  
  {"th","t"},
  {"tt","dd"},
  {"ch","c"},
  {"cc","jj"}, 
  {"kh","k"},
  {"kk","gg"},
  {"ng","ng"},
  {"ss","ss"},
  {"p","b"},
  {"t","d"},
  {"c","j"},
  {"k","g"},					       
  {"m","m"},
  {"n","n"},
  {"s","s"},
  {"w","w"},
  {"l","l"},
  {"r","l"},
  {"y","y"},
  {"h","h"},
  {NULL,NULL}
};






/*
 * find_vowel - return the first substring starting with a vowel 
 */ 

static const mapping * find_vowel(const char * target) {

  const mapping *p;


  for(p = osu_korean_vowels; p->nick != NULL; p++) {
      if(0 == strncmp(target,p->nick,strlen(p->nick)))
	return p;
  }


  return NULL;

}


static const mapping * find_consonant(const char * target) {

  const mapping *p;


  for(p = osu_korean_consonants; p->nick != NULL; p++) {
      if(0 == strncmp(target,p->nick,strlen(p->nick)))
	return p;
  }


  return NULL;

}


/*
 * single hangul need translation too
 */

static const mapping * find_any(const char * target) {
  const mapping * p;
  p = find_vowel(target);
  if(p) 
    return p;
  else
    return find_consonant(target);
}









static void print_syllable(const char * syllable,int hyphenate) {

  const mapping * m;


  if(hyphenate) 
    printf("-");
 
  while(( m = find_consonant(syllable))) {
    printf("%s",m->appendix_n);
    syllable += strlen(m->nick);
  }
  
  if((m=find_vowel(syllable))) {
    printf("%s",m->appendix_n);
    syllable += strlen(m->nick);
  } else {
    printf("__%s__",syllable);
    exit(-1);
  }


  while((m = find_consonant(syllable))) {
    printf("%s",m->appendix_n);
    syllable += strlen(m->nick);
  }


}


static void print_syllable_nospace(const char * syllable) {

  const mapping * m;


  while((m = find_consonant(syllable))) {
    printf("%s",m->appendix_n);
    syllable += strlen(m->nick);
  }
  if((m=find_vowel(syllable))) {
    printf("%s",m->appendix_n);
    syllable += strlen(m->nick);
  } else {
    printf("__%s__",syllable);
    exit(-1);
  }



  while((m = find_consonant(syllable))) {
    printf("%s",m->appendix_n);
    syllable += strlen(m->nick);
  }

}





typedef void (*Printer)(const token tok);

static void print_single_hangul(const char * fragment) {
    const mapping * m;


  while((m = find_any(fragment))) {
    printf("%s ",m->appendix_n);
    fragment += strlen(m->nick);
  }

}


void print_hangul(const token tok) {
  unsigned char c1 = tok.text[0];
  unsigned char c2 = tok.text[1];
  int hyphenate = tok.hyphenate;
  const int offset = 176;


  if(c1 == 164) {

    /*  
  printf("164-%02d:",c2-offset+16 );
  printf("%c%c=",164,c2);
    */

  print_single_hangul(single_hangul[c2-offset+15]);
  /* printf("\n"); */
  
  } else {

  /*
   * Print the character itself
   */


  /* printf(" %c%c = ",c1,c2); */

  /*
   * Print the romanization of the syllable
   */
    if(nospace)
      print_syllable_nospace(romaca[c1-offset][c2-offset+15]);
      else
	print_syllable(romaca[c1-offset][c2-offset+15],hyphenate);
  /* printf("\n"); */
  }
}


void print_roman(const token tok) {
 
  const char * text = tok.text;
  size_t n = tok.size;
  while(n--){
    printf("%c",*text++);
   
    
  }

  
}


void print_whitespace(const token tok) {

  const char * text = tok.text;
  size_t n = tok.size;
  
  while(n--){
    
    printf("%c",*text++);
  }
  
  

}


void dont_print(const token tok) {

  const char * text = tok.text;
  size_t n = tok.size;
  
  while(n--){
    
    text++;
  }
  
  

}




static Printer printers[kNoMoreTypes] = 
{
    print_hangul,/* Text in Hangul */
    dont_print, /* Text in Roman */
    print_whitespace, /* Text that is whitespace */
    dont_print,  /* Punctuation               */
    dont_print  /* XML markup */

};
    



Printer find_printer(token tok) {

  if (tok.type >= 0 && tok.type < kNoMoreTypes)
    return printers[tok.type];
  else
    return NULL;

}




void print_token(token tok) {
  
  
  Printer p = find_printer(tok);
  if(p!=NULL) {
    (*p)(tok);
  } else {
    /* perhaps we should print an error message */
  }
  
}


/*
 * Change: there's an "oe" vowel used in the transcription scheme, added it
 * On this,Mary Beckman  sends the following
 *
 * I just asked Soyoung, and Sun-hee and Misun were here to chime in.	
 * Just as the historically high front rounded vowel had broken into a	
 * sequence of rounded glide followed by unrounded vowel /wi/, the	
 * historically mid front rounded vowel that is transliterated as "oe"	
 * in this table is breaking into /we/, so that, e.g.  17-11: goe		* is (for most younger speakers today) homophonous with 17-43 gwe, and	
 * also with 17-05 gwae (=gwe in our system), and I guess 
 * 17-12:  = goeg g oe g can be rewritten as "gweg", and so on.  
 *
 * On the basis of this, we could provide an optional mapping function
 * that maps "oe" into "we".
 * 
 * Other changes:
 *
 * 19-65 is transcribed as "nyeog", Nick had "nyeong" we have to put "nyeok" 
 * into the table.  
 *
 * 19-72 is transcribed as "nyeok", Nick had "nyeog", to get "nyeok", the 
 * entry should be "nyeokh" 
 *
 * 20-39 is transcribed as neub, to get that we need "neup" in the table 
 * 20-43 is transcribed as neup, to get that we need "neuph" in the table 
 * 32-28 is transcribed as eulb, should be eulp, so we put in "eulph" 
 * 40-08 is transcribed as "hom", Nick had "holm". This is presumably just
 * a mistake.
 */

















/*
 * We're trying to get our phone names to look just like those in 
 * appendix N of Lunde, Nick's are a bit different, no doubt because
 * he (and others) know the Yale system. Following is a table of
 * old and new codes.
 * Differences are
 * (1) where Nick writes the fortis stops as
 *   pp, tt, cc, and kk, * the new romanization has bb, dd, jj, and
 *   gg.  
 * (2) where we had been writing the aspirated stops as ph, th,
 *   ch, and kh, the new romanization has p, t, c, and k 
 *
 * We don't need to do anything for vowels, because they are the same
 * in both systems. Because we are simulating Kyuchul's variety of
 * Korean , our phoneset assigns the same phone to "ae" and "e", but 
 * in the present program we maintain the distinction. The mapping
 * table for vowels is the identity, and exists primarily because
 * we want find_vowel and find_consonant to have the same interface (a
 * property that we need in order to translate Nick's romanizations of
 * single hangul.)
 */



#if 0
static int is_ascii_punctuation(const unsigned char * s) {
  return ispunct(*s);
}
#endif



/*
 * copy_ascii -- deals with copying a sequence
 * starting with an ASCII character to the
 * current token.
 * 
 * We have a simplistic view of XML markup and
 * copy <.*> in one shot. This will fail if there
 * are embedded '>' in the attributes of some tag.
 */

static const unsigned char * copy_ascii(TOKEN tok,const unsigned char * text) {

  const unsigned char * base = tok->text = text;

  if(*text == '<') {
    for( ; *text && *text != '>' && !ispunct(*text); text++) {
      ;
    }
    if(*text){
      /* then we need to print out the > */
      text++;
    }

    tok->size = text-base;
    tok->type = kMarkup; 
  } else {
    
    text++;
    tok->size = 1;
    tok->type = kRoman; 

  }
  return text;
}


static const unsigned char * copy_regular_hangul(TOKEN tok,const unsigned char *text,int just_did_whitespace) {
  const unsigned char * base = text;
  
  
  tok->text = base;
  tok->size = 2;
  tok->type = kHangul;
  tok->hyphenate = !just_did_whitespace;
  return text+2;
}


static const unsigned char * copy_single_hangul(TOKEN tok,const unsigned char * text,int just_did_whitespace) {
  const char * base = text;
  tok->text = base;
  tok->size = 2;
  tok->type = kHangul;
  tok->hyphenate = !just_did_whitespace;
  return text+2;



}


static const unsigned char *copy_spaces(TOKEN tok,const unsigned char * text) {
   
  tok->text = text;
  while(*text && isspace(*text))
    text++;
  tok->size = text - tok->text;

  tok->type = kWhitespace;
  return text;
}




/*
        fgets_unicode
        
	Similar to 'fgets()', but avoids breaking two byte
	characters.


	Return value: as fgets
 

*/

unsigned char * fgets_unicode(unsigned char *line,int n,FILE *fp)	
{
  unsigned char * result = fgets(line,n,fp);
  int nbytes,ch;
  

  if(result != NULL) {
    nbytes = strlen(result);
    ch = result[nbytes-1];
    if(ch & 0x80) {
      result[nbytes-1] = 0;
      ungetc(ch,fp);

      /* Possibly it was the only byte available */
      if(nbytes == 1)
	result = NULL;
      
    }
  }
  
  return result;

}



/*
 * By default this program prints only the hangul
 * part of each file. But command line options
 * can modify the behaviour by altering the 
 * contents of the action table that is used by
 * find_printer.
 */

int process_stream(FILE * fp);



int process_stream(FILE * fp)
{

  static unsigned char line[BUF];   /* we're dealing with 8-bit chars */
  const unsigned char *temp,* limit = line+BUF;
  static token tok;
  int just_did_whitespace;

  while (fgets_unicode(line,BUF,fp) != NULL) {
    
    temp = line;
    while(temp < limit && *temp != '\0') {
      /* first: copy the data corresponding to the
	 next token into tok. It might be any of
	 - a sequence of spaces
	 - a regular hangul character encoded as two bytes
         - a single hangul character
	 - an XML tag
	 - other ascii stuff.
      */
      if (isspace(*temp)) {     
	temp = copy_spaces(&tok,temp);
	just_did_whitespace = 1;
      } else if (*temp >= 176 && *temp < 201 ) {     
	/* it's regular hangul */
	  temp = copy_regular_hangul(&tok,temp,just_did_whitespace);
	  just_did_whitespace = 0;
      } else if (*temp == 164) {                
	/* 
	 * it's a single hangul 
	 */
	temp = copy_single_hangul(&tok,temp,just_did_whitespace);
	just_did_whitespace = 0;  
      } else {
	temp = copy_ascii(&tok,temp);
	just_did_whitespace = 0;  
	
      }
      /* next: print out whatever was copied */
      print_token(tok);
    }
  }
  return 0;
}




int main(int argc,char**argv) {
 
  int arg = 0;

  while(++arg < argc) {
    
    if(strcmp(argv[arg],"--help") == 0) {
      printf("han2phon [options] files\n"
	     "The program breaks the text up into tokens:\n"
	     " - a sequence of spaces (default: printed)\n"
	     "  a regular hangul character (default: romanized)\n"
             " - a single hangul character  (default: romanized)\n"
	     " - an XML tag  (default: not printed)\n"
	     " - other ascii stuff (default: not printed)\n"
	     "Options are:\n"
	     " --nospace suppress spaces\n"
	     " --roman print text in roman e.g. numbers\n"
	     " --markup print markup\n"
	     " --punctuation print punctuation\n"
	     "If no files, take input from standard in\n"
	     "There don't seem to be any punctuation tokens \n"
	     "recognized by the current version, it thinks they\n"
	     "are roman text. We may fix this one day.");

      exit(0);

    }

    if(strcmp(argv[arg],"--nospace") == 0) {
      nospace =1;

    }

    if(strcmp(argv[arg],"--roman") == 0) {
      printers[kRoman] = print_roman;

    }

    if(strcmp(argv[arg],"--markup") == 0) {
      printers[kMarkup] = print_roman;

    }

    if(strcmp(argv[arg],"--punctuation") == 0) {
      printers[kPunct] = print_roman;

    }

    /* arguments that have no - at front are seen as input files */
    if(argv[arg][0] != '-')
      break;
    
  }

  if(argc == arg) {
    process_stream(stdin);
      } else {
	while(arg < argc) {
	  /* work on individual files */
	  /*  fprintf(stderr,"opening %s\n",argv[arg]); */
	  FILE * fp = fopen(argv[arg++],"rb");
	  if(fp == NULL) {
	    fprintf(stderr,"Couldn't open \"%s\" for reading\n",argv[--arg]);
	    exit(-1);
	  }
	  process_stream(fp);
	  assert(fclose(fp) == 0);
	}
      }

  return 0;
}

