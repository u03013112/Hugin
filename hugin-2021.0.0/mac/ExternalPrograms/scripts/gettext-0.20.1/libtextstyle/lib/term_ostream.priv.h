/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

/* Field layout of superclass.  */
#include "ostream.priv.h"

/* Field layout of term_ostream class.  */
struct term_ostream_representation
{
  struct ostream_representation base;
   
  int volatile fd;
  #if HAVE_WINDOWS_CONSOLES
  HANDLE volatile handle;
  bool volatile is_windows_console;
  #endif
  char *filename;
   
                                          
  int max_colors;                         
  int no_color_video;                     
  char * volatile set_a_foreground;       
  char * volatile set_foreground;         
  char * volatile set_a_background;       
  char * volatile set_background;         
  char *orig_pair;                        
  char * volatile enter_bold_mode;        
  char * volatile enter_italics_mode;     
  char *exit_italics_mode;                
  char * volatile enter_underline_mode;   
  char *exit_underline_mode;              
  char *exit_attribute_mode;              
   
  bool volatile supports_foreground;
  bool volatile supports_background;
  colormodel_t volatile colormodel;
  bool volatile supports_weight;
  bool volatile supports_posture;
  bool volatile supports_underline;
   
  const char * volatile restore_colors;
  const char * volatile restore_weight;
  const char * volatile restore_posture;
  const char * volatile restore_underline;
   
  struct term_style_control_data control_data;
   
  #if HAVE_WINDOWS_CONSOLES
  WORD volatile default_console_attributes;
  WORD volatile current_console_attributes;
  #endif
  attributes_t default_attr;          
  attributes_t volatile active_attr;  
  term_color_t volatile active_attr_color;    
  term_color_t volatile active_attr_bgcolor;  
   
  char *buffer;                       
  attributes_t *attrbuffer;           
  size_t buflen;                      
  size_t allocated;                   
  attributes_t curr_attr;             
  attributes_t simp_attr;             
};
