

/* **** **** **** **** **** **** **** **** **** **** **** ****
 * $Id: menus.c,v 2.15 2009/09/06 13:20:10 rda Exp rda $
 *
 * menus.c -  creation of menus for the X/Motif ProofPower
 * Interface
 *
 * menus.c: this file is part of the PPTex system
 *
 * Copyright (c) 2002 Lemma 1 Ltd.
 *
 * See the file LICENSE for your rights to use and change this file.
 *
 * Contact: Rob Arthan < rda@lemma-one.com >
 *
 * **** **** **** **** **** **** **** **** **** **** **** **** */

/* **** **** **** **** **** **** **** **** **** **** **** ****
 * macros:
 * **** **** **** **** **** **** **** **** **** **** **** **** */
#define _menus
/* **** **** **** **** **** **** **** **** **** **** **** ****
 * include files:
 * **** **** **** **** **** **** **** **** **** **** **** **** */

#include "xpp.h"
#include "ctype.h"

static Widget setup_menu_aux(
	Widget	parent,
	int	type,
	char	*menu_title,
	char	menu_mnemonic,
	Bool	tear_off_enabled,
	MenuItem	*items);

static char *sanitise(char *menu_title)
{
	char *res, *p, *q;
	res = XtMalloc(strlen(menu_title) + 1);
	for(p = res, q = menu_title; *q != 0; p += 1, q += 1) {
		*p = isalnum(*q) ? tolower(*q) : '-';
	}
	do {
		*p = 0;
		p -= 1;
	} while (p > res && *p == '-');
	return(res);
}

static void setup_menu_items(
	Widget		menu,
	int		type,
	MenuItem	*items)
{
	Widget w;
	MenuItem	*item;
	XmString	s;

/*
 * Loop round the menu items
 */
	for (item = items; item->label; ++item) {
/*
 * Is this a pull-right submenu?
 */
		if(item->pullright) {
			w = setup_menu_aux(
				menu,
				XmMENU_PULLDOWN,
				item->label,
				item->mnemonic,
				item->pullright_tear_off_enabled,
				item->pullright);
		} else {
			char *name = sanitise(item->label);
			s = XmStringCreateSimple(item->label);
			w = XtVaCreateManagedWidget(
				name,
				*(item->class),
				menu,
				XmNlabelString,	s,
				NULL);
			XmStringFree(s);
			XtFree(name);
		}
/*
 * Is there a mnemonic? (but not for popups)
 */
		if(type == XmMENU_PULLDOWN && item->mnemonic) {
			XtVaSetValues(
				w,
				XmNmnemonic,	item->mnemonic,
				NULL);
		}
/*
 * Is there an accelerator? N.b. accelerator is not allowed for
 * a pull-right submenu.
 */
		if(!item->pullright && item->accelerator) {
			s = XmStringCreateSimple(item->accelerator_text);
			XtVaSetValues(
				w,
				XmNaccelerator,		item->accelerator,
				XmNacceleratorText, 	s,
				NULL);
			XmStringFree(s);
		}
/*
 * Is there a callback? Again not sensible for pull-right submenu
*/
		if(!item->pullright && item->callback) {
			XtAddCallback(
				w, XmNactivateCallback,
				item->callback, item->callback_data);
		}
	}
}

static Widget setup_menu_aux(
	Widget	parent,
	int	type,
	char	*menu_title,
	char	menu_mnemonic,
	Bool	tear_off_enabled,
	MenuItem	*items)
{
	Widget		menu, cascade;
	XmString	lab;
	char *menu_name;
	menu_name = sanitise(menu_title);
	if(type == XmMENU_POPUP) {
/*
 * just create the popup menu:
 */
		menu = XmCreatePopupMenu(parent, menu_name, NULL, 0);
	} else {
/*
 * Create the pulldown menu and its cascade button:
 */
		menu = XmCreatePulldownMenu(parent, menu_name, NULL, 0);
		if(tear_off_enabled) {
			XtVaSetValues(
				menu,
				XmNtearOffModel,	XmTEAR_OFF_ENABLED,
				NULL);
		}
		lab = XmStringCreateSimple(menu_title);
		cascade = XtVaCreateManagedWidget(
			menu_name, xmCascadeButtonGadgetClass, parent,
			XmNsubMenuId,	menu,
			XmNlabelString,	lab,
			XmNmnemonic,	(KeySym)menu_mnemonic,
			NULL);
		XmStringFree(lab);
	}

	setup_menu_items(menu, type, items);

	XtFree(menu_name);
#ifdef LISTWIDGETS
	register_shell(menu);
#endif
	return type == XmMENU_POPUP ? menu : cascade;

}

Widget setup_menu(
	Widget  parent,
	int	type,
	char    *menu_title,
	char    menu_mnemonic,
	Boolean tear_off_enabled,
	MenuItem        *items)
{
	if(type == XmMENU_POPUP) {
		return
			setup_menu_aux(
				parent, type, menu_title,
				menu_mnemonic, tear_off_enabled, items);
	} else {
		Widget cascade, menu;
		cascade = setup_menu_aux(
				parent, type, menu_title,
				menu_mnemonic, tear_off_enabled, items);
		XtVaGetValues(
				cascade,
				XmNsubMenuId,   &menu,
				NULL);
		return menu;
	}
}

void resetup_menu(
	Widget		menu,
	int		type,
	MenuItem	*items)
{
	Widget *btns;
	Cardinal num_btns;
	int i;

	XtVaGetValues(menu, XmNchildren, &btns,
		XmNnumChildren, &num_btns, NULL);

	for(i = 0; i < num_btns; ++i) {
		XtUnmanageChild(btns[i]);
		XtDestroyWidget(btns[i]);
	}

	setup_menu_items(menu, type, items);	
}
