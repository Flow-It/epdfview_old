// ePDFView - A lightweight PDF Viewer.
// Copyright (C) 2006, 2007, 2009 Emma's Software.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <epdfview.h>
#include "PageView.h"

using namespace ePDFView;

// Constants
static gint PAGE_VIEW_PADDING = 12;
static gint SCROLL_PAGE_DRAG_LENGTH = 50;
static const gint PIXBUF_BITS_PER_SAMPLE = 8;

// Forwards declarations.
static gboolean page_view_button_press_cb (GtkWidget *, GdkEventButton *,
                                           gpointer);
static gboolean page_view_button_release_cb (GtkWidget *, GdkEventButton *, 
                                             gpointer);
static gboolean page_view_mouse_motion_cb (GtkWidget *, GdkEventMotion *,
                                           gpointer);
static void page_view_get_scrollbars_size (GtkWidget *,
                                           gint *width, gint *height);
static void page_view_resized_cb (GtkWidget *, GtkAllocation *, gpointer);
static gboolean page_view_scrolled_cb (GtkWidget *, GdkEventScroll *, gpointer);
static gboolean page_view_keypress_cb (GtkWidget *, GdkEventKey *, gpointer);

static void gdkpixbuf_invert(GdkPixbuf *pb) {//krogan edit
	int width, height, rowlength, n_channels;
	guchar *pixels, *p;

	n_channels = gdk_pixbuf_get_n_channels (pb);

	g_assert (gdk_pixbuf_get_colorspace (pb) == GDK_COLORSPACE_RGB);
	g_assert (gdk_pixbuf_get_bits_per_sample (pb) == 8);
	g_assert (gdk_pixbuf_get_has_alpha (pb));
	g_assert (n_channels == 4);

	width = gdk_pixbuf_get_width (pb);
	height = gdk_pixbuf_get_height (pb);

	rowlength = width*n_channels;
	
	pixels = gdk_pixbuf_get_pixels (pb);
	
	int i;
	int max = rowlength*height;
	for(i=0;i<max;i+=n_channels) {
		p = pixels + i;
		p[0] = 255 - p[0];
		p[1] = 255 - p[1];
		p[2] = 255 - p[2];
	}
}

PageView::PageView ():
    IPageView ()
{
    // The initial cursor is normal.
    m_CurrentCursor = PAGE_VIEW_CURSOR_NORMAL;

    // Create the scrolled window where the page image will be.
    m_PageScroll = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (m_PageScroll),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);

    // The actual page image.
    m_PageImage = gtk_image_new ();
    gtk_misc_set_padding (GTK_MISC (m_PageImage), 
                          PAGE_VIEW_PADDING, PAGE_VIEW_PADDING);

    // I want to be able to drag the page with the left mouse
    // button, because that will make possible to move the page
    // with an stylus on a PDA. Later I'll need this to implement the
    // document's links. The GtkImage widget doesn't have a
    // GdkWindow, so I have to add the event box that will receive
    // the mouse events.
    m_EventBox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (m_EventBox), m_PageImage);
    gtk_scrolled_window_add_with_viewport (
            GTK_SCROLLED_WINDOW (m_PageScroll), m_EventBox);

    gtk_widget_show_all (m_PageScroll);
    
    invertColorToggle = 0;
    hasShownAPage = 0;
}

PageView::~PageView ()
{
}

void //krogan edit
PageView::setInvertColorToggle(char on)
{
	invertColorToggle = on;
}

gdouble
PageView::getHorizontalScroll ()
{
    GtkAdjustment *hAdjustment = gtk_scrolled_window_get_hadjustment (
            GTK_SCROLLED_WINDOW (m_PageScroll));
    return gtk_adjustment_get_value (hAdjustment);
}

void 
PageView::getSize (gint *width, gint *height)
{
    g_assert (NULL != width && "Tried to save the width to a NULL pointer.");
    g_assert (NULL != height && "Tried to save the height to a NULL pointer.");

    gint vScrollSize = 0;
    gint hScrollSize = 0;
    page_view_get_scrollbars_size (m_PageScroll, &vScrollSize, &hScrollSize);
    GtkAllocation alloc;
    gtk_widget_get_allocation (m_PageScroll, &alloc);
    *width = alloc.width - vScrollSize;
    *height = alloc.height - hScrollSize;
}

gdouble
PageView::getVerticalScroll ()
{
    GtkAdjustment *vAdjustment = gtk_scrolled_window_get_vadjustment (
            GTK_SCROLLED_WINDOW (m_PageScroll));
    return gtk_adjustment_get_value (vAdjustment);
}

void
PageView::makeRectangleVisible (DocumentRectangle &rect, gdouble scale)
{
    gdouble margin = 5 * scale;

    // Calculate the horizontal adjustment.
    GtkAdjustment *hAdjustment = gtk_scrolled_window_get_hadjustment (
            GTK_SCROLLED_WINDOW (m_PageScroll));

    gdouble hvalue = gtk_adjustment_get_value (hAdjustment);
    gdouble hlower = gtk_adjustment_get_lower (hAdjustment);
    gdouble hupper = gtk_adjustment_get_upper (hAdjustment);
    gdouble hpagesize = gtk_adjustment_get_page_size (hAdjustment);

    gdouble realX1 = rect.getX1 () * scale;
    gdouble realX2 = rect.getX2 () * scale;
    gdouble docX1 = getHorizontalScroll () - PAGE_VIEW_PADDING;
    gdouble docX2 = docX1 + hpagesize;

    gdouble dx = 0.0;
    if ( realX1 < docX1 )
    {
        dx = (realX1 - docX1 - margin);
    }
    else if ( realX2 > docX2 )
    {
        dx = (realX2 - docX2 + margin);
    }

    gtk_adjustment_set_value (GTK_ADJUSTMENT (hAdjustment),
            CLAMP (hvalue + dx, hlower, hupper - hpagesize));

    // Calculate the vertical adjustment.
    GtkAdjustment *vAdjustment = gtk_scrolled_window_get_vadjustment (
            GTK_SCROLLED_WINDOW (m_PageScroll));

    gdouble vvalue = gtk_adjustment_get_value (vAdjustment);
    gdouble vlower = gtk_adjustment_get_lower (vAdjustment);
    gdouble vupper = gtk_adjustment_get_upper (vAdjustment);
    gdouble vpagesize = gtk_adjustment_get_page_size (vAdjustment);

    gdouble realY1 = rect.getY1 () * scale;
    gdouble realY2 = rect.getY2 () * scale;
    gdouble docY1 = getVerticalScroll () - PAGE_VIEW_PADDING;
    gdouble docY2 = docY1 + vpagesize;

    gdouble dy = 0;
    if ( realY1 < docY1 )
    {
        dy = (realY1 - docY1 - margin);
    }
    else if ( realY2 > docY2 )
    {
        dy = (realY2 - docY2 + margin);
    }

    gtk_adjustment_set_value (GTK_ADJUSTMENT (vAdjustment),
            CLAMP (vvalue + dy, vlower, vupper - vpagesize));
}

void
PageView::resizePage (gint width, gint height)
{
    GdkPixbuf *originalPage = gtk_image_get_pixbuf (GTK_IMAGE (m_PageImage));
    if ( NULL != originalPage )
    {
        GdkPixbuf *scaledPage = gdk_pixbuf_scale_simple (originalPage,
                                                         width, height,
                                                         GDK_INTERP_NEAREST);
        if ( NULL != scaledPage )
        {
            gtk_image_set_from_pixbuf (GTK_IMAGE (m_PageImage), scaledPage);
            g_object_unref (scaledPage);
        }
    }
}

void
PageView::setCursor (PageCursor cursorType)
{
    if ( cursorType != m_CurrentCursor )
    {
        GdkCursor *cursor = NULL;
        switch (cursorType)
        {
            case PAGE_VIEW_CURSOR_SELECT_TEXT:
                cursor = gdk_cursor_new (GDK_XTERM);
                break;
            case PAGE_VIEW_CURSOR_LINK:
                cursor = gdk_cursor_new (GDK_HAND2);
                break;
            case PAGE_VIEW_CURSOR_DRAG:
                cursor = gdk_cursor_new (GDK_FLEUR);
                break;
            default:
                cursor = NULL;
        }

	if ( NULL != m_EventBox )
        {
	    GdkWindow *win = gtk_widget_get_window (m_EventBox);
            if ( NULL != win )
	    {
                gdk_window_set_cursor (win, cursor);
	    }
        }
        if ( NULL != cursor )
        {
            gdk_cursor_unref (cursor);
        }
        gdk_flush ();
        m_CurrentCursor = cursorType;
    }
}

void
PageView::setPresenter (PagePter *pter)
{
    IPageView::setPresenter (pter);

    // When resizing.
    g_signal_connect (G_OBJECT (m_PageScroll), "size-allocate",
                      G_CALLBACK (page_view_resized_cb), pter);
    // When scrolling.
    g_signal_connect (G_OBJECT (m_PageScroll), "scroll-event",
                      G_CALLBACK (page_view_scrolled_cb), pter);
    g_signal_connect (G_OBJECT (m_PageScroll), "key-press-event",
                      G_CALLBACK (page_view_keypress_cb), pter);

    // And connect the motion, button press and release events.
    gtk_widget_add_events (m_EventBox, GDK_POINTER_MOTION_MASK |
                                       GDK_POINTER_MOTION_HINT_MASK |
                                       GDK_BUTTON_PRESS_MASK |
                                       GDK_BUTTON_RELEASE_MASK);
    g_signal_connect (G_OBJECT (m_EventBox), "button-press-event",
                      G_CALLBACK (page_view_button_press_cb), this);
    g_signal_connect (G_OBJECT (m_EventBox), "motion-notify-event",
                      G_CALLBACK (page_view_mouse_motion_cb), this);
    g_signal_connect (G_OBJECT (m_EventBox), "button-release-event",
                      G_CALLBACK (page_view_button_release_cb), this);
}

void
PageView::scrollPage (gdouble scrollX, gdouble scrollY, gint dx, gint dy)
{
    GtkAllocation alloc;
    gtk_widget_get_allocation (m_PageImage, &alloc);

    /* if the page cannot scroll and i'm dragging bottom to up, or left to right, 
     i will go to the next page. viceversa previous page */
    GtkAdjustment *hAdjustment = gtk_scrolled_window_get_hadjustment (
            GTK_SCROLLED_WINDOW (m_PageScroll));

    gdouble hlower = gtk_adjustment_get_lower (hAdjustment);
    gdouble hupper = gtk_adjustment_get_upper (hAdjustment);
    gdouble hpagesize = gtk_adjustment_get_page_size (hAdjustment);
    gdouble hAdjValue = hpagesize * (gdouble)dx / alloc.width;
    gtk_adjustment_set_value (hAdjustment,
            CLAMP (scrollX - hAdjValue, hlower, hupper - hpagesize));

    GtkAdjustment *vAdjustment = gtk_scrolled_window_get_vadjustment (
            GTK_SCROLLED_WINDOW (m_PageScroll));

    gdouble vlower = gtk_adjustment_get_lower (vAdjustment);
    gdouble vupper = gtk_adjustment_get_upper (vAdjustment);
    gdouble vpagesize = gtk_adjustment_get_page_size (vAdjustment);
    gdouble vAdjValue = vpagesize * (gdouble)dy / alloc.height;
    gtk_adjustment_set_value (vAdjustment,
            CLAMP (scrollY - vAdjValue, vlower, vupper - vpagesize));
    
    /* if the page cannot scroll and i'm dragging bottom to up, or left to right, 
       I will go to the next page. viceversa previous page */
    if ( (scrollY == (vupper - vpagesize) && dy < (-SCROLL_PAGE_DRAG_LENGTH) ) ||
        (scrollX == (hupper - hpagesize) && dx < (-SCROLL_PAGE_DRAG_LENGTH)) )
    {
        m_Pter->scrollToNextPage();
        m_Pter->mouseButtonReleased(1);
    }
    else if( (scrollY == vlower && dy > SCROLL_PAGE_DRAG_LENGTH) ||
        (scrollX == hlower && dx > SCROLL_PAGE_DRAG_LENGTH) )
    {
        m_Pter->scrollToPreviousPage();
        m_Pter->mouseButtonReleased(1);
    }
}

void 
PageView::showPage (DocumentPage *page, PageScroll scroll)
{
	hasShownAPage = 1;
	lastPageShown = page;
	lastScroll = scroll;
	
    gtk_image_set_from_pixbuf (GTK_IMAGE (m_PageImage), NULL);
    GdkPixbuf *pixbuf = getPixbufFromPage (page);
    
    if(invertColorToggle) { gdkpixbuf_invert(pixbuf); }
    
    gtk_image_set_from_pixbuf (GTK_IMAGE (m_PageImage), pixbuf);
    g_object_unref (pixbuf);
    // Set the vertical scroll to the specified.
    if ( PAGE_SCROLL_NONE != scroll )
    {
        GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment (
                                    GTK_SCROLLED_WINDOW (m_PageScroll));

	gdouble val;
        if ( PAGE_SCROLL_START == scroll )
        {
	    val = gtk_adjustment_get_lower (adjustment);
            gtk_adjustment_set_value (adjustment, val);
        }
        else if ( PAGE_SCROLL_END == scroll )
        {
	    val = gtk_adjustment_get_upper (adjustment);
            gtk_adjustment_set_value (adjustment, val);
        }
    }
}

void
PageView::tryReShowPage()
{
	if(hasShownAPage) {
		showPage(lastPageShown, lastScroll);
	}
}

////////////////////////////////////////////////////////////////
// GTK+ Functions.
////////////////////////////////////////////////////////////////

GtkWidget *
PageView::getTopWidget ()
{
    return m_PageScroll;
}

void
PageView::getPagePosition (gint widgetX, gint widgetY, gint *pageX, gint *pageY)
{
    g_assert ( NULL != pageX && "Tried to save the page's X to NULL.");
    g_assert ( NULL != pageY && "Tried to save the page's Y to NULL.");

    // Since the page is centered on the GtkImage widget, we need to
    // get the current widget size and the current image size to know
    // how many widget space is being used for padding.
    gint horizontalPadding = PAGE_VIEW_PADDING;
    gint verticalPadding = PAGE_VIEW_PADDING;
    GdkPixbuf *page = gtk_image_get_pixbuf (GTK_IMAGE (m_PageImage));
    if ( NULL != page )
    {
	GtkAllocation alloc;
	gtk_widget_get_allocation (m_PageImage, &alloc);
        horizontalPadding = (alloc.width - gdk_pixbuf_get_width (page)) / 2;
        verticalPadding = (alloc.height - gdk_pixbuf_get_height (page)) / 2;
    }

    *pageX = widgetX - horizontalPadding + (gint)getHorizontalScroll ();
    *pageY = widgetY - verticalPadding + (gint)getVerticalScroll ();
}

///
/// @brief Creates a new GdkPixbuf from a given DocumentPage.
///
/// @param page The DocumentPage to transform to a GdkPixbuf.
/// @return The resultant GdkPixbuf.
///
GdkPixbuf *
PageView::getPixbufFromPage (DocumentPage *page)
{
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data (page->getData (),
            GDK_COLORSPACE_RGB, page->hasAlpha(), PIXBUF_BITS_PER_SAMPLE,
            page->getWidth (), page->getHeight (),
            page->getRowStride (), NULL, NULL);
    GdkPixbuf *finalPixbuf = gdk_pixbuf_copy (pixbuf);
    g_object_unref (pixbuf);

    return finalPixbuf;
}

////////////////////////////////////////////////////////////////
// Callbacks
////////////////////////////////////////////////////////////////

///
/// @brief A mouse button has been pressed.
///
gboolean
page_view_button_press_cb (GtkWidget *widget, GdkEventButton *event,
                           gpointer data)
{
    g_assert ( NULL != data && "The data is NULL.");
    PageView *view = (PageView *)data;

    gint event_x;
    gint event_y;
    gtk_widget_get_pointer (view->getTopWidget (), &event_x, &event_y);
    gint x;
    gint y;
    view->getPagePosition (event_x, event_y, &x, &y);
    view->getPresenter ()->mouseButtonPressed (event->button, event->state, x, y);

    gtk_widget_grab_focus(view->getTopWidget());
    
    return TRUE;
}

///
/// @brief A mouse button has been released.
///
gboolean
page_view_button_release_cb (GtkWidget *widget, GdkEventButton *event,
                             gpointer data)
{
    g_assert ( NULL != data && "The data is NULL.");

    PageView *view = (PageView *)data;
    view->getPresenter ()->mouseButtonReleased (event->button);
    
    return TRUE;
}

///
/// @brief The page view is being dragged.
///
gboolean
page_view_mouse_motion_cb (GtkWidget *widget, GdkEventMotion *event,
                           gpointer data)
{
    g_assert ( NULL != data && "The data is NULL.");
    PageView *view = (PageView *)data;

    gint event_x;
    gint event_y;
    gtk_widget_get_pointer (view->getTopWidget (), &event_x, &event_y);
    gint x;
    gint y;
    view->getPagePosition (event_x, event_y, &x, &y);
    view->getPresenter ()->mouseMoved (x, y);

    return TRUE;
}


void
page_view_get_scrollbars_size (GtkWidget *widget, gint *width, gint *height)
{
    g_assert (NULL != width && "Tried to save the width to a NULL pointer.");
    g_assert (NULL != height && "Tried to save the height to a NULL pointer.");

    gint borderWidth = widget->style->xthickness;
    gint borderHeight = widget->style->ythickness;
    if ( GTK_SHADOW_NONE !=
            gtk_scrolled_window_get_shadow_type (GTK_SCROLLED_WINDOW (widget)) )
    {
        borderWidth += widget->style->xthickness;
        borderHeight += widget->style->ythickness;
    }

    gint scrollBarSpacing = 0;
    gtk_widget_style_get (widget, "scrollbar-spacing", &scrollBarSpacing, NULL);

    GtkWidget *vScrollBar = GTK_SCROLLED_WINDOW (widget)->vscrollbar;
    GtkWidget *hScrollBar = GTK_SCROLLED_WINDOW (widget)->hscrollbar;
    GtkAllocation valloc, halloc;
    gtk_widget_get_allocation (vScrollBar, &valloc);
    gtk_widget_get_allocation (hScrollBar, &halloc);
    *width = valloc.width +
             (PAGE_VIEW_PADDING + borderWidth) * 2 + scrollBarSpacing;

    *height = halloc.height +
              (PAGE_VIEW_PADDING + borderHeight) * 2 + scrollBarSpacing;
}

///
/// @brief The page view has been resized.
///
static void
page_view_resized_cb (GtkWidget *widget, GtkAllocation *allocation,
                                  gpointer data)
{
    g_assert ( NULL != data && "The data parameter is NULL.");

    gint vScrollSize = 0;
    gint hScrollSize = 0;
    page_view_get_scrollbars_size (widget, &vScrollSize, &hScrollSize);

    gint width = allocation->width - vScrollSize;
    gint height = allocation->height - hScrollSize;

    PagePter *pter = (PagePter *)data;
    pter->viewResized (width, height);
}

///
/// @brief The page view has been scrolled.
///
/// This only happens when the user uses the mouse wheel.
///
static gboolean
page_view_scrolled_cb (GtkWidget *widget, GdkEventScroll *event, gpointer data)
{
    g_assert ( NULL != data && "The data parameter is NULL.");

    PagePter *pter = (PagePter *)data;
    GtkAdjustment *adjustment = 
        gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (widget));
    gdouble position = gtk_adjustment_get_value (adjustment);
    gdouble lower = gtk_adjustment_get_lower (adjustment);
    gdouble upper = gtk_adjustment_get_upper (adjustment);
    gdouble pagesize = gtk_adjustment_get_page_size (adjustment);

    if ( GDK_SCROLL_UP == event->direction && position == lower )
    {
        pter->scrollToPreviousPage ();
        return TRUE;
    }
    else if ( GDK_SCROLL_DOWN == event->direction &&
              position == ( upper - pagesize) )
    {
        pter->scrollToNextPage ();
        return TRUE;
    }

    return FALSE;
}

///
/// @brief A key was pressed.
///
static gboolean
page_view_keypress_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    g_assert ( NULL != data && "The data parameter is NULL.");

    GtkScrollType direction;
    gboolean horizontal = FALSE;
    gboolean returnValue = TRUE;
    PagePter *pter = (PagePter *)data;

    GtkAdjustment *hadjustment = 
        gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (widget));
    gdouble hposition = gtk_adjustment_get_value (hadjustment);
    gdouble hlower = gtk_adjustment_get_lower (hadjustment);
    gdouble hupper = gtk_adjustment_get_upper (hadjustment);
    gdouble hpagesize = gtk_adjustment_get_page_size (hadjustment);

    GtkAdjustment *vadjustment = 
        gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (widget));
    gdouble vposition = gtk_adjustment_get_value (vadjustment);
    gdouble vlower = gtk_adjustment_get_lower (vadjustment);
    gdouble vupper = gtk_adjustment_get_upper (vadjustment);
    gdouble vpagesize = gtk_adjustment_get_page_size (vadjustment);

    if ( event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK) )
    {
        return FALSE;
    }

    switch ( event->keyval )
    {
        case GDK_KEY_Left:
        case GDK_KEY_KP_Left:
        case GDK_KEY_h:
            if ( hposition == hlower )
            {
                pter->scrollToPreviousPage ();
                return TRUE;
            }
            direction = GTK_SCROLL_STEP_LEFT;
            horizontal = TRUE;
            break;

        case GDK_KEY_Right:
        case GDK_KEY_KP_Right:
        case GDK_KEY_l:
            if ( hposition == ( hupper - hpagesize) )
            {
                pter->scrollToNextPage ();
                return TRUE;
            }
            horizontal = TRUE;
            direction = GTK_SCROLL_STEP_RIGHT;
            break;

        case GDK_KEY_Up:
        case GDK_KEY_KP_Up:
        case GDK_KEY_k:
            if ( vposition == vlower )
            {
                pter->scrollToPreviousPage ();
                return TRUE;
            }
            direction = GTK_SCROLL_STEP_UP;
            break;

        case GDK_KEY_Down:
        case GDK_KEY_KP_Down:
        case GDK_KEY_j:
            if ( vposition == ( vupper - vpagesize) )
            {
                pter->scrollToNextPage ();
                return TRUE;
            }
            direction = GTK_SCROLL_STEP_DOWN;
            break;

        case GDK_KEY_Page_Up:
        case GDK_KEY_KP_Page_Up:
            if ( vposition == vlower )
            {
                pter->scrollToPreviousPage ();
                return TRUE;
            }
            direction = GTK_SCROLL_PAGE_UP;
            break;

        case GDK_KEY_space:
        case GDK_KEY_KP_Space:
        case GDK_KEY_Page_Down:
        case GDK_KEY_KP_Page_Down:
            if ( vposition == ( vupper - vpagesize) )
            {
                pter->scrollToNextPage ();
                return TRUE;
            }
            direction = GTK_SCROLL_PAGE_DOWN;
            break;

       case GDK_KEY_Home:
       case GDK_KEY_KP_Home:
            direction = GTK_SCROLL_START;
            break;

       case GDK_KEY_End:
       case GDK_KEY_KP_End:
            direction = GTK_SCROLL_END;
            break;

       case GDK_KEY_Return:
       case GDK_KEY_KP_Enter:
            pter->scrollToNextPage ();
            direction = GTK_SCROLL_START;
            break;

       case GDK_KEY_BackSpace:
            pter->scrollToPreviousPage ();
            direction = GTK_SCROLL_START;
            break;

       default:
            return FALSE;
    }

    g_signal_emit_by_name(G_OBJECT(widget), "scroll-child",
                          direction, horizontal, &returnValue);
    return returnValue;
}
