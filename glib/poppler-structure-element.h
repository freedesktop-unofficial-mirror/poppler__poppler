/* poppler-structure-element.h: glib interface to poppler
 *
 * Copyright (C) 2013 Igalia S.L.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __POPPLER_STRUCTURE_ELEMENT_H__
#define __POPPLER_STRUCTURE_ELEMENT_H__

#include <glib-object.h>
#include "poppler.h"

G_BEGIN_DECLS

#define POPPLER_TYPE_STRUCTURE_ELEMENT    (poppler_structure_element_get_type ())
#define POPPLER_STRUCTURE_ELEMENT(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), POPPLER_TYPE_STRUCTURE_ELEMENT, PopplerStructureElement))
#define POPPLER_IS_STRUCTURE_ELEMENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), POPPLER_TYPE_STRUCTURE_ELEMENT))

/**
 * PopplerStructureElementKind:
 */
typedef enum {
  POPPLER_STRUCTURE_ELEMENT_UNKNOWN,
  POPPLER_STRUCTURE_ELEMENT_CONTENT,
  POPPLER_STRUCTURE_ELEMENT_OBJECT_REFERENCE,
  POPPLER_STRUCTURE_ELEMENT_DOCUMENT,
  POPPLER_STRUCTURE_ELEMENT_PART,
  POPPLER_STRUCTURE_ELEMENT_ARTICLE,
  POPPLER_STRUCTURE_ELEMENT_SECTION,
  POPPLER_STRUCTURE_ELEMENT_DIV,
  POPPLER_STRUCTURE_ELEMENT_SPAN,
  POPPLER_STRUCTURE_ELEMENT_QUOTE,
  POPPLER_STRUCTURE_ELEMENT_NOTE,
  POPPLER_STRUCTURE_ELEMENT_REFERENCE,
  POPPLER_STRUCTURE_ELEMENT_BIBENTRY,
  POPPLER_STRUCTURE_ELEMENT_CODE,
  POPPLER_STRUCTURE_ELEMENT_LINK,
  POPPLER_STRUCTURE_ELEMENT_ANNOT,
  POPPLER_STRUCTURE_ELEMENT_RUBY,
  POPPLER_STRUCTURE_ELEMENT_WARICHU,
  POPPLER_STRUCTURE_ELEMENT_BLOCKQUOTE,
  POPPLER_STRUCTURE_ELEMENT_CAPTION,
  POPPLER_STRUCTURE_ELEMENT_NONSTRUCT,
  POPPLER_STRUCTURE_ELEMENT_TOC,
  POPPLER_STRUCTURE_ELEMENT_TOC_ITEM,
  POPPLER_STRUCTURE_ELEMENT_INDEX,
  POPPLER_STRUCTURE_ELEMENT_PRIVATE,
  POPPLER_STRUCTURE_ELEMENT_PARAGRAPH,
  POPPLER_STRUCTURE_ELEMENT_HEADING,
  POPPLER_STRUCTURE_ELEMENT_HEADING_1,
  POPPLER_STRUCTURE_ELEMENT_HEADING_2,
  POPPLER_STRUCTURE_ELEMENT_HEADING_3,
  POPPLER_STRUCTURE_ELEMENT_HEADING_4,
  POPPLER_STRUCTURE_ELEMENT_HEADING_5,
  POPPLER_STRUCTURE_ELEMENT_HEADING_6,
  POPPLER_STRUCTURE_ELEMENT_LIST,
  POPPLER_STRUCTURE_ELEMENT_LIST_ITEM,
  POPPLER_STRUCTURE_ELEMENT_LIST_LABEL,
  POPPLER_STRUCTURE_ELEMENT_LIST_BODY,
  POPPLER_STRUCTURE_ELEMENT_TABLE,
  POPPLER_STRUCTURE_ELEMENT_TABLE_ROW,
  POPPLER_STRUCTURE_ELEMENT_TABLE_HEADING,
  POPPLER_STRUCTURE_ELEMENT_TABLE_DATA,
  POPPLER_STRUCTURE_ELEMENT_TABLE_HEADER,
  POPPLER_STRUCTURE_ELEMENT_TABLE_FOOTER,
  POPPLER_STRUCTURE_ELEMENT_TABLE_BODY,
  POPPLER_STRUCTURE_ELEMENT_FIGURE,
  POPPLER_STRUCTURE_ELEMENT_FORMULA,
  POPPLER_STRUCTURE_ELEMENT_FORM,
} PopplerStructureElementKind;

typedef struct _PopplerTextSpan PopplerTextSpan;
struct _PopplerTextSpan {
  gchar *text;
  gchar *font_name;
  gchar *link_target;
  guint  flags;
  guint  color; /* 0x00RRGGBB */
};

enum {
  POPPLER_TEXT_SPAN_FIXED_WIDTH = (1 << 0),
  POPPLER_TEXT_SPAN_SERIF_FONT  = (1 << 1),
  POPPLER_TEXT_SPAN_ITALIC      = (1 << 2),
  POPPLER_TEXT_SPAN_BOLD        = (1 << 3),
  POPPLER_TEXT_SPAN_LINK        = (1 << 4),
  POPPLER_TEXT_SPAN_COLOR       = (1 << 5),
  POPPLER_TEXT_SPAN_FONT        = (1 << 6),
};


GType                        poppler_structure_element_get_type                   (void) G_GNUC_CONST;
PopplerStructureElementKind  poppler_structure_element_get_kind                   (PopplerStructureElement  *poppler_structure_element);
gint                         poppler_structure_element_get_page                   (PopplerStructureElement  *poppler_structure_element);
gboolean                     poppler_structure_element_is_content                 (PopplerStructureElement  *poppler_structure_element);
gboolean                     poppler_structure_element_is_inline                  (PopplerStructureElement  *poppler_structure_element);
gboolean                     poppler_structure_element_is_block                   (PopplerStructureElement  *poppler_structure_element);
const gchar                 *poppler_structure_element_get_id                     (PopplerStructureElement  *poppler_structure_element);
const gchar                 *poppler_structure_element_get_title                  (PopplerStructureElement  *poppler_structure_element);
const gchar                 *poppler_structure_element_get_abbreviation           (PopplerStructureElement  *poppler_structure_element);
const gchar                 *poppler_structure_element_get_language               (PopplerStructureElement  *poppler_structure_element);
const gchar                 *poppler_structure_element_get_text                   (PopplerStructureElement  *poppler_structure_element,
                                                                                   gboolean                  recursive);
GList                       *poppler_structure_element_get_text_spans             (PopplerStructureElement  *poppler_structure_element,
                                                                                   gboolean                  recursive);
const gchar                 *poppler_structure_element_get_alt_text               (PopplerStructureElement  *poppler_structure_element);
const gchar                 *poppler_structure_element_get_actual_text            (PopplerStructureElement  *poppler_structure_element);
PopplerFormField            *poppler_structure_element_get_form_field             (PopplerStructureElement  *poppler_structure_element);
PopplerFormFieldMapping     *poppler_structure_element_get_form_field_mapping     (PopplerStructureElement  *poppler_structure_element);

#define POPPLER_TYPE_STRUCTURE_ELEMENT_ITER                                       (poppler_structure_element_iter_get_type ())
GType                        poppler_structure_element_iter_get_type              (void) G_GNUC_CONST;
PopplerStructureElementIter *poppler_structure_element_iter_new                   (PopplerDocument             *poppler_document);
PopplerStructureElementIter *poppler_structure_element_iter_get_child             (PopplerStructureElementIter *parent);
PopplerStructureElementIter *poppler_structure_element_iter_copy                  (PopplerStructureElementIter *iter);
PopplerStructureElement     *poppler_structure_element_iter_get_element           (PopplerStructureElementIter *iter);
gboolean                     poppler_structure_element_iter_next                  (PopplerStructureElementIter *iter);
void                         poppler_structure_element_iter_free                  (PopplerStructureElementIter *iter);

gboolean                     poppler_text_span_is_fixed_width                     (PopplerTextSpan *poppler_text_span);
gboolean                     poppler_text_span_is_serif_font                      (PopplerTextSpan *poppler_text_span);
gboolean                     poppler_text_span_is_bold                            (PopplerTextSpan *poppler_text_span);
gboolean                     poppler_text_span_is_link                            (PopplerTextSpan *poppler_text_span);

G_END_DECLS

#endif /* !__POPPLER_STRUCTURE_ELEMENT_H__ */
