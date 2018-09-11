#include <err.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "xdraw.h"
#include "histogram.h"

oxbarui_t*
ui_create(
      const char *wname,
      int         x,
      int         y,
      int         width,
      int         height,
      int         padding,
      double      fontpt,
      const char *font,
      const char *bgcolor,
      const char *fgcolor
      )
{
   oxbarui_t *ui = malloc(sizeof(oxbarui_t));
   if (NULL == ui)
      err(1, "%s: couldn't malloc ui", __FUNCTION__);

   ui->xinfo = malloc(sizeof(xinfo_t));
   if (NULL == ui->xinfo)
      err(1, "%s: couldn't malloc xinfo", __FUNCTION__);

   ui->bgcolor = strdup(bgcolor);
   ui->fgcolor = strdup(fgcolor);
   if (NULL == ui->bgcolor || NULL == ui->fgcolor)
      err(1, "%s: strdup failed", __FUNCTION__);

   /* XXX These need to be done in a specific order */
   xcore_setup_x_connection_screen_visual(ui->xinfo);

   if (-1 == height)
      height = (uint32_t)(ceil(fontpt + (2 * padding)));

   if (-1 == y)
      y = ui->xinfo->display_height - height;

   if (-1 == width)
      width = ui->xinfo->display_width;

   ui->xinfo->padding = padding;

   xcore_setup_x_window(
         ui->xinfo,
         wname,
         x, y,
         width, height);
   xcore_setup_x_wm_hints(ui->xinfo);
   xcore_setup_cairo(ui->xinfo);
   xcore_setup_xfont(ui->xinfo, font, fontpt);

   /* now map the window & do an initial paint */
   xcb_map_window(ui->xinfo->xcon, ui->xinfo->xwindow);

   /* TODO configurable */
   ui->widget_padding = 10.0;
   ui->small_space = 5.0;

   return ui;
}

void
ui_destroy(oxbarui_t *ui)
{
   free(ui->bgcolor);
   free(ui->fgcolor);
   xcore_destroy(ui->xinfo);
   free(ui->xinfo);
   free(ui);
}

void
ui_clear(oxbarui_t *ui)
{
   xdraw_clear_all(ui->xinfo, ui->bgcolor);
   ui->xcurrent = ui->xinfo->padding;
}

void
ui_flush(oxbarui_t *ui)
{
   xdraw_flush(ui->xinfo);
}

void
ui_widget_battery_draw(
      oxbarui_t *ui,
      bool        plugged_in,
      double      charge_pct,
      const char *str_charge_pct,
      int         minutes_remaining,
      const char *str_time_remaining)
{
   static const char *colors[] = {
      "dc322f",
      "859900"
   };

   double startx = ui->xcurrent;
   double y = ui->xinfo->fontpt + ui->xinfo->padding;

   ui->xcurrent += xdraw_text(
         ui->xinfo,
         plugged_in ? ui->fgcolor : "dc322f",
         ui->xcurrent,
         y,
         plugged_in ? "AC:" : "BAT:");
   ui->xcurrent += ui->small_space;
   ui->xcurrent += xdraw_vertical_stack(
         ui->xinfo,
         ui->xcurrent,
         7, 2,
         colors,
         (double[]){100.0 - charge_pct, charge_pct});
   ui->xcurrent += ui->small_space;
   ui->xcurrent += xdraw_text(
         ui->xinfo,
         ui->fgcolor,
         ui->xcurrent,
         y,
         str_charge_pct);

   if (-1 != minutes_remaining) {
      ui->xcurrent += ui->small_space;
      ui->xcurrent += xdraw_text(
            ui->xinfo,
            ui->fgcolor,
            ui->xcurrent,
            y,
            str_time_remaining);
   }
   xdraw_hline(ui->xinfo, "b58900b2", ui->xinfo->padding, startx, ui->xcurrent);
   ui->xcurrent += ui->widget_padding;
}

void
ui_widget_volume_draw(
      oxbarui_t  *ui,
      double      left_pct,
      double      right_pct,
      const char *str_left_pct,
      const char *str_right_pct)
{
   static const char *colors[] = {
      "dc322f",
      "859900"
   };
   double startx = ui->xcurrent;
   double y = ui->xinfo->fontpt + ui->xinfo->padding;

   ui->xcurrent += xdraw_text(ui->xinfo, ui->fgcolor, ui->xcurrent, y, "VOLUME:");
   ui->xcurrent += ui->small_space;
   if (left_pct == right_pct) {
      ui->xcurrent += xdraw_vertical_stack(ui->xinfo, ui->xcurrent, 7, 2,
            colors,
            (double[]){100.0 - left_pct, left_pct});
      ui->xcurrent += ui->small_space;
      ui->xcurrent += xdraw_text(ui->xinfo, ui->fgcolor, ui->xcurrent, y, str_left_pct);
   } else {
      ui->xcurrent += xdraw_text(ui->xinfo, ui->fgcolor, ui->xcurrent, y, str_left_pct);
      ui->xcurrent += ui->small_space;
      ui->xcurrent += xdraw_text(ui->xinfo, ui->fgcolor, ui->xcurrent, y, str_right_pct);
   }
   xdraw_hline(ui->xinfo, "cb4b16b2", ui->xinfo->padding, startx, ui->xcurrent);
   ui->xcurrent += ui->widget_padding;
}

void
ui_widget_nprocs_draw(
      oxbarui_t  *ui,
      const char *str_nprocs)
{
   double startx = ui->xcurrent;
   double y = ui->xinfo->fontpt + ui->xinfo->padding;

   ui->xcurrent += xdraw_text(ui->xinfo, ui->fgcolor, ui->xcurrent, y, "#PROCS:");
   ui->xcurrent += ui->small_space;
   ui->xcurrent += xdraw_text(ui->xinfo, ui->fgcolor, ui->xcurrent, y, str_nprocs);
   xdraw_hline(ui->xinfo, "dc322fb2", ui->xinfo->padding, startx, ui->xcurrent);
   ui->xcurrent += ui->widget_padding;
}

void
ui_widget_memory_draw(
      oxbarui_t  *ui,
      double      free_pct,
      double      total_pct,
      double      active_pct,
      const char *str_free,
      const char *str_total,
      const char *str_active)
{
   static const char *colors[] = {
      "859900",
      "bbbb00",
      "dc322f"
   };
   double startx = ui->xcurrent;
   double y = ui->xinfo->fontpt + ui->xinfo->padding;

   static histogram_t *histogram = NULL;
   if (NULL == histogram)
      histogram = histogram_init(60, 3);

   histogram_update(histogram, (double[]) {
         free_pct,
         total_pct,
         active_pct
         });

   ui->xcurrent += xdraw_text(ui->xinfo, ui->fgcolor, ui->xcurrent, y, "MEMORY:");
   ui->xcurrent += ui->small_space;
   ui->xcurrent += xdraw_histogram(ui->xinfo, ui->xcurrent, colors, histogram);
   ui->xcurrent += ui->small_space;
   ui->xcurrent += xdraw_text(ui->xinfo, "dc322f", ui->xcurrent, y, str_active);
   ui->xcurrent += ui->small_space;
   ui->xcurrent += xdraw_text(ui->xinfo, ui->fgcolor, ui->xcurrent, y, "active");
   ui->xcurrent += ui->small_space;
   ui->xcurrent += xdraw_text(ui->xinfo, "b58900", ui->xcurrent, y, str_total);
   ui->xcurrent += ui->small_space;
   ui->xcurrent += xdraw_text(ui->xinfo, ui->fgcolor, ui->xcurrent, y, "total");
   ui->xcurrent += ui->small_space;
   ui->xcurrent += xdraw_text(ui->xinfo, "859900", ui->xcurrent, y, str_free);
   ui->xcurrent += ui->small_space;
   ui->xcurrent += xdraw_text(ui->xinfo, ui->fgcolor, ui->xcurrent, y, "free");
   xdraw_hline(ui->xinfo, "d33682b2", ui->xinfo->padding, startx, ui->xcurrent);

   ui->xcurrent += ui->widget_padding;
}

void
ui_widget_cpus_draw(
      oxbarui_t  *ui,
      cpus_t     *cpus)
{
   static const char *colors[] = {
      "859900",   /* idle */
      "d33682",   /* interrupt */
      "b58900",   /* sys  */
      "2aa198",   /* nice */
      "dc322f"    /* user */
   };
   double startx = ui->xcurrent;
   double y = ui->xinfo->fontpt + ui->xinfo->padding;
   int i;

   static histogram_t **hist_cpu = NULL;
   if (NULL == hist_cpu) {
      hist_cpu = calloc(cpus->ncpu, sizeof(histogram_t*));
      if (NULL == hist_cpu)
         err(1, "%s: calloc hist_cpu failed", __FUNCTION__);

      for (i = 0; i < cpus->ncpu; i++)
         hist_cpu[i] = histogram_init(60, CPUSTATES);
   }

   ui->xcurrent += xdraw_text(ui->xinfo, ui->fgcolor, ui->xcurrent, y, "CPUS:");
   ui->xcurrent += ui->small_space;
   for (i = 0; i < cpus->ncpu; i++) {
      histogram_update(hist_cpu[i], (double[]) {
            cpus->cpus[i].percentages[CP_IDLE],
            cpus->cpus[i].percentages[CP_INTR],
            cpus->cpus[i].percentages[CP_SYS],
            cpus->cpus[i].percentages[CP_NICE],
            cpus->cpus[i].percentages[CP_USER]
            });
      ui->xcurrent += xdraw_histogram(ui->xinfo, ui->xcurrent, colors, hist_cpu[i]);
      ui->xcurrent += ui->small_space;
      ui->xcurrent += xdraw_text(ui->xinfo, ui->fgcolor, ui->xcurrent, y,
            CPUS.cpus[i].str_percentages[CP_IDLE]);
      ui->xcurrent += ui->small_space;
   }
   xdraw_hline(ui->xinfo, "6c71c4b2", ui->xinfo->padding, startx, ui->xcurrent);

   ui->xcurrent += ui->widget_padding;
}
