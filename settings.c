#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/limits.h>

#include "widgets.h"
#include "settings.h"

void
widget_set_hdcolor(const char *widget_name, char *color)
{
   for (size_t i = 0; i < NWIDGETS; i++) {
      if (0 == strncasecmp(widget_name, WIDGETS[i].name, strlen(widget_name))) {
         WIDGETS[i].hdcolor = color;
      }
   }
}

void
settings_load_defaults(settings_t *s)
{
   s->display.x = 0;
   s->display.y = -1;
   s->display.w = -1;
   s->display.h = -1;
   s->display.padding_top = 10;
   s->display.widget_spacing = 15;
   s->display.wmname  = strdup(getprogname());
   s->display.font    = strdup("DejaVu Sans 16px");
   s->display.bgcolor = strdup("1c1c1cdd");
   s->display.fgcolor = strdup("93a1a1");

   s->battery.hdcolor             = strdup("b58900");
   widget_set_hdcolor("battery", s->battery.hdcolor);
   s->battery.fgcolor_unplugged   = strdup("dc322f");
   s->battery.chart_width         = 7;
   s->battery.chart_bgcolor = strdup("dc322f");
   s->battery.chart_pgcolor = strdup("859900");

   s->volume.hdcolor       = strdup("cb4b16");
   widget_set_hdcolor("volume", s->volume.hdcolor);
   s->volume.chart_width   = 7;
   s->volume.chart_bgcolor = strdup("dc322f");
   s->volume.chart_pgcolor = strdup("859900");

   s->nprocs.hdcolor = strdup("dc322f");
   widget_set_hdcolor("nprocs", s->nprocs.hdcolor);

   s->memory.hdcolor             = strdup("d33682");
   widget_set_hdcolor("memory", s->memory.hdcolor);
   s->memory.chart_bgcolor       = strdup("555555");
   s->memory.chart_color_free    = strdup("859900");
   s->memory.chart_color_total   = strdup("bbbb00");
   s->memory.chart_color_active  = strdup("dc322f");

   s->cpus.hdcolor               = strdup("6c71c4");
   widget_set_hdcolor("cpus", s->cpus.hdcolor);
   s->cpus.chart_bgcolor         = strdup("555555");
   s->cpus.chart_color_sys       = strdup("ff0000");
   s->cpus.chart_color_interrupt = strdup("ffff00");
   s->cpus.chart_color_user      = strdup("0000ff");
   s->cpus.chart_color_nice      = strdup("ff00ff");
   s->cpus.chart_color_spin      = strdup("00ffff");
   s->cpus.chart_color_idle      = strdup("859900");

   s->network.hdcolor                        = strdup("268bd2");
   widget_set_hdcolor("net", s->network.hdcolor);
   s->network.chart_bgcolor                  = strdup("555555");
   s->network.inbound_chart_color_bgcolor    = strdup("859900");
   s->network.inbound_chart_color_pgcolor    = strdup("157ad2");
   s->network.outbound_chart_color_bgcolor   = strdup("859900");
   s->network.outbound_chart_color_pgcolor   = strdup("dc322f");

   s->time.hdcolor = strdup("859900");
   widget_set_hdcolor("time", s->time.hdcolor);
   s->time.format  = strdup("%a %d %b %Y  %I:%M:%S %p");
}

/* TODO Create a settings_free() (or refactor to make easier)
 * I should have this - but this is cumbersome given the above setup.
 * Go to array based? Then every settings retrieval in the draw loop is an
 * array lookup. Bleh. I had a version of this started, but seemed silly.
void
settings_free(settings_t *s)
{
   free(s->display.wmname);
   free(s->display.bgcolor);
   free(s->display.fgcolor);
   free(s->display.font);

   free(s->battery.hdcolor);
   free(s->battery.str_plugged_in);
   free(s->battery.str_unplugged);
   ...
}
*/

/* TODO Support a config file
void
settings_parse_config(settings_t *s, const char *file)
{
   if (s != NULL)
      errx(1, "%s: not supported yet (%s)", __FUNCTION__, file);
}
*/

#define SET_STRING_VALUE(name) \
   if (0 == strncasecmp( #name , property, strlen( #name ))) { \
      if (NULL == (s->name = strdup(value))) \
         err(1, "%s: strdup failed for key %s", __FUNCTION__, property); \
         \
      return; \
   }


#define SET_INT_VALUE(name) \
   if (0 == strncasecmp( #name , property , strlen( #name ))) { \
      s->name = strtonum(value, 0, INT_MAX, &errstr); \
      if (errstr) \
         errx(1, "%s: bad value %s for key %s", __FUNCTION__, value, property); \
      \
      return; \
   }

void
settings_set_keyvalue(settings_t *s, char *keyvalue)
{
   /* XXX contract: keyvalue is like "<widget>.<property>=<value>" */
   const char *errstr;
   char       *property = keyvalue;
   char       *dot      = strstr(keyvalue, ".");
   char       *eq       = strstr(keyvalue, "=");

   if (NULL == eq || NULL == dot)
      errx(1, "bad -S");

   char *value = eq + 1;
   *eq = '\0';

   /* display */
   SET_STRING_VALUE(display.wmname);
   SET_STRING_VALUE(display.font);
   SET_STRING_VALUE(display.bgcolor);
   SET_STRING_VALUE(display.fgcolor);

   /* battery */
   SET_STRING_VALUE(battery.hdcolor);
   widget_set_hdcolor("battery", s->battery.hdcolor);
   SET_STRING_VALUE(battery.fgcolor_unplugged);
   SET_INT_VALUE(battery.chart_width);
   SET_STRING_VALUE(battery.chart_bgcolor);
   SET_STRING_VALUE(battery.chart_pgcolor);

   /* volume */
   SET_STRING_VALUE(volume.hdcolor);
   widget_set_hdcolor("volume", s->volume.hdcolor);
   SET_INT_VALUE(volume.chart_width);
   SET_STRING_VALUE(volume.chart_bgcolor);
   SET_STRING_VALUE(volume.chart_pgcolor);

   /* nprocs */
   SET_STRING_VALUE(nprocs.hdcolor);
   widget_set_hdcolor("nprocs", s->nprocs.hdcolor);

   /* memory */
   SET_STRING_VALUE(memory.hdcolor);
   widget_set_hdcolor("memory", s->memory.hdcolor);
   SET_STRING_VALUE(memory.chart_color_free);
   SET_STRING_VALUE(memory.chart_color_total);
   SET_STRING_VALUE(memory.chart_color_active);

   /* cpus */
   SET_STRING_VALUE(cpus.hdcolor);
   widget_set_hdcolor("cpus", s->cpus.hdcolor);
   SET_STRING_VALUE(cpus.chart_color_idle);
   SET_STRING_VALUE(cpus.chart_color_user);
   SET_STRING_VALUE(cpus.chart_color_sys);
   SET_STRING_VALUE(cpus.chart_color_nice);
   SET_STRING_VALUE(cpus.chart_color_interrupt);

   /* network */
   SET_STRING_VALUE(network.hdcolor);
   widget_set_hdcolor("network", s->network.hdcolor);
   SET_STRING_VALUE(network.inbound_chart_color_bgcolor);
   SET_STRING_VALUE(network.inbound_chart_color_pgcolor);
   SET_STRING_VALUE(network.outbound_chart_color_bgcolor);
   SET_STRING_VALUE(network.outbound_chart_color_pgcolor);

   /* time */
   SET_STRING_VALUE(time.hdcolor);
   widget_set_hdcolor("time", s->time.hdcolor);

   /* unknown key! */
   errx(1, "unknown key '%s''", keyvalue);
}

void
usage()
{
   printf("usage: to do!\n"); /* TODO */
   exit(1);
}

void
settings_parse_cmdline(settings_t *s, int argc, char *argv[])
{
   const char *errstr;
   char *keyvalue;
   int ch;

   while (-1 != (ch = getopt(argc, argv, "x:y:w:h:p:s:f:S:t:"))) {
      switch (ch) {
      case 'x':
         s->display.x = strtonum(optarg, 0, INT_MAX, &errstr);
         if (errstr)
            errx(1, "illegal x value '%s': %s", optarg, errstr);
         break;
      case 'y':
         s->display.y = strtonum(optarg, -1, INT_MAX, &errstr);
         if (errstr)
            errx(1, "illegal y value '%s': %s", optarg, errstr);
         break;
      case 'w':
         s->display.w = strtonum(optarg, -1, INT_MAX, &errstr);
         if (errstr)
            errx(1, "illegal w value '%s': %s", optarg, errstr);
         break;
      case 'h':
         s->display.h = strtonum(optarg, -1, INT_MAX, &errstr);
         if (errstr)
            errx(1, "illegal h value '%s': %s", optarg, errstr);
         break;
      case 'p':
         s->display.padding_top = strtonum(optarg, 0, INT_MAX, &errstr);
         if (errstr)
            errx(1, "illegal p value '%s': %s", optarg, errstr);
         break;
      case 's':
         s->display.widget_spacing = strtonum(optarg, 0, INT_MAX, &errstr);
         if (errstr)
            errx(1, "illegal s value '%s': %s", optarg, errstr);
         break;
      case 'S':
         if (NULL == (keyvalue = strdup(optarg)))
            err(1, "strdup failed for font");

         settings_set_keyvalue(s, keyvalue);
         free(keyvalue);
         break;
      case 'f':
         free(s->display.font);
         s->display.font = strdup(optarg);
         if (NULL == s->display.font)
            err(1, "strdup failed for font");
         break;
      case 't':
         free(s->time.format);
         s->time.format = strdup(optarg);
         if (NULL == s->time.format)
            err(1, "strdup failed for time format");
         break;
      default:
         usage();
      }
   }

   argc -= optind;
   /* argv += optind; uneeded */

   if (argc)
      usage();
}
