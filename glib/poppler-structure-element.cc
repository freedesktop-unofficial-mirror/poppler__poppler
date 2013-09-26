/* poppler-structure.cc: glib interface to poppler
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

#include "config.h"

#ifndef __GI_SCANNER__
#include <StructTreeRoot.h>
#include <StructElement.h>
#include <GlobalParams.h>
#include <UnicodeMap.h>
#endif /* !__GI_SCANNER__ */

#include "poppler.h"
#include "poppler-private.h"
#include "poppler-structure-element.h"


/**
 * SECTION:poppler-structure-element
 * @short_description: Document structure element.
 * @title: PopplerStructureElement
 * @see_also: #PopplerStructure
 *
 * Instances of #PopplerStructureElement are used to describe the structure
 * of a #PopplerDocument. To access the elements in the structure of the
 * document, first use poppler_document_get_structure() to obtain its
 * #PopplerStructure, and then use poppler_structure_get_n_children()
 * and poppler_structure_get_child() to enumerate the top level elements.
 */

static PopplerStructureElement *
  _poppler_structure_element_new (PopplerDocument *document, StructElement *element);


struct _PopplerStructureElementIter
{
  PopplerDocument *document;
  union {
    StructElement  *elem;
    StructTreeRoot *root;
  };
  gboolean is_root;
  unsigned index;
};

POPPLER_DEFINE_BOXED_TYPE (PopplerStructureElementIter,
                           poppler_structure_element_iter,
                           poppler_structure_element_iter_copy,
                           poppler_structure_element_iter_free)

/**
 * poppler_structure_element_iter_copy:
 * @iter: a #PopplerStructureElementIter
 *
 * Creates a new #PopplerStructureElementIter as a copy of @iter. The
 * returned value must be freed with poppler_structure_element_iter_free().
 *
 * Return value: a new #PopplerStructureElementIter
 */
PopplerStructureElementIter *
poppler_structure_element_iter_copy (PopplerStructureElementIter *iter)
{
  PopplerStructureElementIter *new_iter;

  g_return_val_if_fail (iter != NULL, NULL);

  new_iter = g_slice_dup (PopplerStructureElementIter, iter);
  new_iter->document = (PopplerDocument *) g_object_ref (new_iter->document);

  return new_iter;
}

/**
 * poppler_structure_element_iter_free:
 * @iter: a #PopplerStructureElementIter
 *
 * Frees @iter.
 */
void
poppler_structure_element_iter_free (PopplerStructureElementIter *iter)
{
  if (G_UNLIKELY (iter == NULL))
    return;

  g_object_unref (iter->document);
  g_slice_free (PopplerStructureElementIter, iter);
}

/**
 * poppler_structure_element_iter_new:
 * @poppler_document: a #PopplerDocument.
 *
 * Returns the root #PopplerStructureElementIter for @document, or %NULL. The
 * returned value must be freed with * poppler_structure_element_iter_free().
 *
 * Documents may have an associated structure tree &mdashmostly, Tagged-PDF
 * compliant documents&mdash; which can be used to obtain information about
 * the document structure and its contents. Each node in the tree contains
 * a #PopplerStructureElement.
 *
 * Here is a simple example that walks the whole tree:
 *
 * <informalexample><programlisting>
 * static void
 * walk_structure (PopplerStructureElementIter *iter)
 * {
 *   do {
 *     /<!-- -->* Get the element and do something with it *<!-- -->/
 *     PopplerStructureElementIter *child = poppler_structure_element_iter_get_child (iter);
 *     if (child)
 *       walk_structure (child);
 *     poppler_structure_element_iter_free (child);
 *   } while (poppler_structure_element_iter_next (iter));
 * }
 * ...
 * {
 *   iter = poppler_structure_element_iter_new (document);
 *   walk_structure (iter);
 *   poppler_structure_element_iter_free (iter);
 * }
 * </programlisting></informalexample>
 *
 * Return value: (transfer full): a new #PopplerStructureElementIter
 */
PopplerStructureElementIter *
poppler_structure_element_iter_new (PopplerDocument *poppler_document)
{
  PopplerStructureElementIter *iter;
  StructTreeRoot *root;

  g_return_val_if_fail (POPPLER_IS_DOCUMENT (poppler_document), NULL);

  root = poppler_document->doc->getStructTreeRoot ();
  if (root == NULL)
    return NULL;

  if (root->getNumElements () == 0)
    return NULL;

  iter = g_slice_new0 (PopplerStructureElementIter);
  iter->document = (PopplerDocument *) g_object_ref (poppler_document);
  iter->is_root  = TRUE;
  iter->root     = root;

  return iter;
}

/**
 * poppler_structure_element_iter_next:
 * @iter: a #PopplerStructureElementIter
 *
 * Sets @iter to point to the next structure element at the current level
 * of the tree, if valid. See poppler_structure_element_iter_new() for more
 * information.
 *
 * Return value: %TRUE, if @iter was set to the next structure element
 */
gboolean
poppler_structure_element_iter_next (PopplerStructureElementIter *iter)
{
  unsigned elements;

  g_return_val_if_fail (iter != NULL, FALSE);

  elements = iter->is_root
    ? iter->root->getNumElements ()
    : iter->elem->getNumElements ();

  return ++iter->index < elements;
}

/**
 * poppler_structure_element_iter_get_element:
 * @iter: a #PopplerStructureElementIter
 *
 * Returns the #PopplerStructureElementIter associated with @iter.
 *
 * Return value: (transfer full): a new #PopplerStructureElementIter
 */
PopplerStructureElement *
poppler_structure_element_iter_get_element (PopplerStructureElementIter *iter)
{
  StructElement *elem;

  g_return_val_if_fail (iter != NULL, NULL);

  elem = iter->is_root
    ? iter->root->getElement (iter->index)
    : iter->elem->getElement (iter->index);

  return _poppler_structure_element_new (iter->document, elem);
}

/**
 * poppler_structure_element_iter_get_child:
 * @parent: a #PopplerStructureElementIter
 *
 * Return value: a new #PopplerStructureElementIter
 */
PopplerStructureElementIter *
poppler_structure_element_iter_get_child (PopplerStructureElementIter *parent)
{
  StructElement *elem;

  g_return_val_if_fail (parent != NULL, NULL);

  elem = parent->is_root
    ? parent->root->getElement (parent->index)
    : parent->elem->getElement (parent->index);

  if (elem->getNumElements () > 0)
    {
      PopplerStructureElementIter *child = g_slice_new0 (PopplerStructureElementIter);
      child->document = (PopplerDocument *) g_object_ref (parent->document);
      child->elem = elem;
      return child;
    }

  return NULL;
}


static PopplerStructureElementKind
_poppler_structelement_type_to_poppler_structure_element_kind (StructElement::Type type)
{
  switch (type)
    {
      case StructElement::Unknown:
        return POPPLER_STRUCTURE_ELEMENT_UNKNOWN;
      case StructElement::MCID:
        return POPPLER_STRUCTURE_ELEMENT_CONTENT;
      case StructElement::OBJR:
        return POPPLER_STRUCTURE_ELEMENT_OBJECT_REFERENCE;
      case StructElement::Document:
        return POPPLER_STRUCTURE_ELEMENT_DOCUMENT;
      case StructElement::Part:
        return POPPLER_STRUCTURE_ELEMENT_PART;
      case StructElement::Sect:
        return POPPLER_STRUCTURE_ELEMENT_SECTION;
      case StructElement::Div:
        return POPPLER_STRUCTURE_ELEMENT_DIV;
      case StructElement::Span:
        return POPPLER_STRUCTURE_ELEMENT_SPAN;
      case StructElement::Quote:
        return POPPLER_STRUCTURE_ELEMENT_QUOTE;
      case StructElement::Note:
        return POPPLER_STRUCTURE_ELEMENT_NOTE;
      case StructElement::Reference:
        return POPPLER_STRUCTURE_ELEMENT_REFERENCE;
      case StructElement::BibEntry:
        return POPPLER_STRUCTURE_ELEMENT_BIBENTRY;
      case StructElement::Code:
        return POPPLER_STRUCTURE_ELEMENT_CODE;
      case StructElement::Link:
        return POPPLER_STRUCTURE_ELEMENT_LINK;
      case StructElement::Annot:
        return POPPLER_STRUCTURE_ELEMENT_ANNOT;
      case StructElement::Ruby:
        return POPPLER_STRUCTURE_ELEMENT_RUBY;
      case StructElement::Warichu:
        return POPPLER_STRUCTURE_ELEMENT_WARICHU;
      case StructElement::BlockQuote:
        return POPPLER_STRUCTURE_ELEMENT_BLOCKQUOTE;
      case StructElement::Caption:
        return POPPLER_STRUCTURE_ELEMENT_CAPTION;
      case StructElement::NonStruct:
        return POPPLER_STRUCTURE_ELEMENT_NONSTRUCT;
      case StructElement::TOC:
        return POPPLER_STRUCTURE_ELEMENT_TOC;
      case StructElement::TOCI:
        return POPPLER_STRUCTURE_ELEMENT_TOC_ITEM;
      case StructElement::Index:
        return POPPLER_STRUCTURE_ELEMENT_INDEX;
      case StructElement::Private:
        return POPPLER_STRUCTURE_ELEMENT_PRIVATE;
      case StructElement::P:
        return POPPLER_STRUCTURE_ELEMENT_PARAGRAPH;
      case StructElement::H:
        return POPPLER_STRUCTURE_ELEMENT_HEADING;
      case StructElement::H1:
        return POPPLER_STRUCTURE_ELEMENT_HEADING_1;
      case StructElement::H2:
        return POPPLER_STRUCTURE_ELEMENT_HEADING_2;
      case StructElement::H3:
        return POPPLER_STRUCTURE_ELEMENT_HEADING_3;
      case StructElement::H4:
        return POPPLER_STRUCTURE_ELEMENT_HEADING_4;
      case StructElement::H5:
        return POPPLER_STRUCTURE_ELEMENT_HEADING_5;
      case StructElement::H6:
        return POPPLER_STRUCTURE_ELEMENT_HEADING_6;
      case StructElement::L:
        return POPPLER_STRUCTURE_ELEMENT_LIST;
      case StructElement::LI:
        return POPPLER_STRUCTURE_ELEMENT_LIST_ITEM;
      case StructElement::Lbl:
        return POPPLER_STRUCTURE_ELEMENT_LIST_LABEL;
      case StructElement::LBody:
        return POPPLER_STRUCTURE_ELEMENT_LIST_BODY;
      case StructElement::Table:
        return POPPLER_STRUCTURE_ELEMENT_TABLE;
      case StructElement::TR:
        return POPPLER_STRUCTURE_ELEMENT_TABLE_ROW;
      case StructElement::TH:
        return POPPLER_STRUCTURE_ELEMENT_TABLE_HEADING;
      case StructElement::TD:
        return POPPLER_STRUCTURE_ELEMENT_TABLE_DATA;
      case StructElement::THead:
        return POPPLER_STRUCTURE_ELEMENT_TABLE_HEADER;
      case StructElement::TFoot:
        return POPPLER_STRUCTURE_ELEMENT_TABLE_FOOTER;
      case StructElement::TBody:
        return POPPLER_STRUCTURE_ELEMENT_TABLE_BODY;
      case StructElement::Figure:
        return POPPLER_STRUCTURE_ELEMENT_FIGURE;
      case StructElement::Formula:
        return POPPLER_STRUCTURE_ELEMENT_FORMULA;
      case StructElement::Form:
        return POPPLER_STRUCTURE_ELEMENT_FORM;
      default:
        g_assert_not_reached ();
    }
}


static void _poppler_text_span_free (gpointer data)
{
  PopplerTextSpan *span = (PopplerTextSpan*) data;
  g_free (span->text);
  g_free (span->font_name);
  g_free (span->link_target);
  g_slice_free (PopplerTextSpan, data);
}


typedef struct _PopplerStructureElementClass PopplerStructureElementClass;
struct _PopplerStructureElementClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE (PopplerStructureElement, poppler_structure_element, G_TYPE_OBJECT);


static PopplerStructureElement *
_poppler_structure_element_new (PopplerDocument *document, StructElement *element)
{
  PopplerStructureElement *poppler_structure_element;

  g_assert (POPPLER_IS_DOCUMENT (document));
  g_assert (element);

  poppler_structure_element = (PopplerStructureElement *) g_object_new (POPPLER_TYPE_STRUCTURE_ELEMENT, NULL, NULL);
  poppler_structure_element->document  = (PopplerDocument *) g_object_ref (document);
  poppler_structure_element->elem      = element;

  return poppler_structure_element;
}


static void
poppler_structure_element_init (PopplerStructureElement *poppler_structure_element)
{
}


static void
poppler_structure_element_finalize (GObject *object)
{
  PopplerStructureElement *poppler_structure_element = POPPLER_STRUCTURE_ELEMENT (object);

  /* poppler_structure_element->elem is owned by the StructTreeRoot */
  g_free (poppler_structure_element->language);
  g_free (poppler_structure_element->text_r);
  g_free (poppler_structure_element->text);
  g_free (poppler_structure_element->title);
  g_free (poppler_structure_element->id);
  g_object_unref (poppler_structure_element->document);
  g_list_free_full (poppler_structure_element->text_spans, _poppler_text_span_free);

  G_OBJECT_CLASS (poppler_structure_element_parent_class)->finalize (object);
}


static void
poppler_structure_element_class_init (PopplerStructureElementClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = poppler_structure_element_finalize;
}


/**
 * poppler_structure_element_get_kind:
 * @poppler_structure_element: A #PopplerStructureElement
 *
 * Return value: A #PopplerStructureElementKind value.
 */
PopplerStructureElementKind
poppler_structure_element_get_kind (PopplerStructureElement *poppler_structure_element)
{
  g_return_val_if_fail (POPPLER_IS_STRUCTURE_ELEMENT (poppler_structure_element), POPPLER_STRUCTURE_ELEMENT_UNKNOWN);
  g_assert (poppler_structure_element->elem);

  return _poppler_structelement_type_to_poppler_structure_element_kind (poppler_structure_element->elem->getType ());
}

/**
 * poppler_structure_element_get_page:
 * @poppler_structure_element: A #PopplerStructureElement
 *
 * Return value: Number of the page that contains the element, of
 *    <code>-1</code> if not defined.
 */
gint
poppler_structure_element_get_page (PopplerStructureElement *poppler_structure_element)
{
  g_return_val_if_fail (POPPLER_IS_STRUCTURE_ELEMENT (poppler_structure_element), POPPLER_STRUCTURE_ELEMENT_UNKNOWN);
  g_assert (poppler_structure_element->elem);

  Ref ref;
  if (poppler_structure_element->elem->getPageRef (ref))
    {
      return poppler_structure_element->document->doc->findPage(ref.num, ref.gen) - 1;
    }

  return -1;
}

/**
 * poppler_structure_element_is_content:
 * @poppler_structure_element: A #PopplerStructureElement
 *
 * Checks whether an element is actual document content.
 *
 * Return value: Whether the element is content.
 */
gboolean
poppler_structure_element_is_content (PopplerStructureElement *poppler_structure_element)
{
  g_return_val_if_fail (POPPLER_IS_STRUCTURE_ELEMENT (poppler_structure_element), FALSE);
  g_assert (poppler_structure_element->elem);

  return poppler_structure_element->elem->isContent ();
}

/**
 * poppler_structure_element_is_inline:
 * @poppler_structure_element: A #PopplerStructureElement
 *
 * Checks whether an element is an inline element.
 *
 * Return value: Whether the element is inline.
 */
gboolean
poppler_structure_element_is_inline (PopplerStructureElement *poppler_structure_element)
{
  g_return_val_if_fail (POPPLER_IS_STRUCTURE_ELEMENT (poppler_structure_element), FALSE);
  g_assert (poppler_structure_element->elem);

  return poppler_structure_element->elem->isInline ();
}

/**
 * poppler_structure_element_is_block:
 * @poppler_structure_element: A #PopplerStructureElement
 *
 * Checks whether an element is a block element.
 *
 * Return value: Whether the element is block.
 */
gboolean
poppler_structure_element_is_block (PopplerStructureElement *poppler_structure_element)
{
  g_return_val_if_fail (POPPLER_IS_STRUCTURE_ELEMENT (poppler_structure_element), FALSE);
  g_assert (poppler_structure_element->elem);

  return poppler_structure_element->elem->isBlock ();
}

/**
 * poppler_structure_element_get_id:
 * @poppler_structure_element: A #PopplerStructureElement
 *
 * Return value: (transfer none): The identifier of the element (if
 *    defined), or %NULL.
 */
const gchar*
poppler_structure_element_get_id (PopplerStructureElement *poppler_structure_element)
{
  g_return_val_if_fail (POPPLER_IS_STRUCTURE_ELEMENT (poppler_structure_element), NULL);
  g_assert (poppler_structure_element->elem);

  if (!poppler_structure_element->id && poppler_structure_element->elem->getID ())
    poppler_structure_element->id = _poppler_goo_string_to_utf8 (poppler_structure_element->elem->getID ());

  return poppler_structure_element->id;
}

/**
 * poppler_structure_element_get_title:
 * @poppler_structure_element: A #PopplerStructureElement
 *
 * Return value: (transfer none): The title of the element (if defined),
 *    or %NULL.
 */
const gchar*
poppler_structure_element_get_title (PopplerStructureElement *poppler_structure_element)
{
  g_return_val_if_fail (POPPLER_IS_STRUCTURE_ELEMENT (poppler_structure_element), NULL);
  g_assert (poppler_structure_element->elem);

  if (!poppler_structure_element->title && poppler_structure_element->elem->getTitle ())
    poppler_structure_element->title = _poppler_goo_string_to_utf8 (poppler_structure_element->elem->getTitle ());

  return poppler_structure_element->title;
}

/**
 * popppler_structure_element_get_abbreviation:
 * @poppler_structure_element: A #PopplerStructureElement
 *
 * Acronyms and abbreviations contained in elements of type
 * #POPPLER_STRUCTURE_ELEMENT_SPAN may have an associated expanded
 * text form, which can be retrieved using this function.
 *
 * Return value: (transfer none): Text of the expanded abbreviation, if the
 *    element text is an abbreviation or acronym.
 */
const gchar*
poppler_structure_element_get_abbreviation (PopplerStructureElement *poppler_structure_element)
{
  g_return_val_if_fail (POPPLER_IS_STRUCTURE_ELEMENT (poppler_structure_element), NULL);
  g_assert (poppler_structure_element->elem);

  if (poppler_structure_element->elem->getType () != StructElement::Span)
    return NULL;

  if (!poppler_structure_element->text_abbrev && poppler_structure_element->elem->getExpandedAbbr ())
    poppler_structure_element->text_abbrev = _poppler_goo_string_to_utf8 (poppler_structure_element->elem->getExpandedAbbr ());

  return poppler_structure_element->text_abbrev;
}

/**
 * poppler_structure_element_get_language:
 * @poppler_structure_element: A #PopplerStructureElement
 *
 * Return value: (transfer none): language and country code, in two-letter
 *    ISO format, e.g. <code>en_US</code>, or %NULL if not defined.
 */
const gchar*
poppler_structure_element_get_language (PopplerStructureElement *poppler_structure_element)
{
  g_return_val_if_fail (POPPLER_IS_STRUCTURE_ELEMENT (poppler_structure_element), NULL);
  g_assert (poppler_structure_element->elem);

  if (!poppler_structure_element->language && poppler_structure_element->elem->getLanguage ())
    poppler_structure_element->language = _poppler_goo_string_to_utf8 (poppler_structure_element->elem->getLanguage ());

  return poppler_structure_element->language;
}

/**
 * poppler_structure_element_get_alt_text:
 * @poppler_structure_element: A #PopplerStructureElement
 *
 * Obtains the “alternate” text representation of the element (and its child
 * elements). This is mostly used for non-text elements like images and
 * figures, to specify a textual description of the element.
 *
 * Note that for elements containing proper text, the function
 * poppler_structure_element_get_text() must be used instead.
 *
 * Return value: (transfer none): The alternate text representation for the
 *    element, or %NULL if not defined.
 */
const gchar*
poppler_structure_element_get_alt_text (PopplerStructureElement *poppler_structure_element)
{
  g_return_val_if_fail (POPPLER_IS_STRUCTURE_ELEMENT (poppler_structure_element), NULL);
  g_assert (poppler_structure_element->elem);

  if (!poppler_structure_element->alt_text && poppler_structure_element->elem->getAltText ())
    {
      GooString *s = poppler_structure_element->elem->getAltText ();
      if (s)
        poppler_structure_element->alt_text = _poppler_goo_string_to_utf8 (s);
      delete s;
    }

  return poppler_structure_element->alt_text;
}

/**
 * poppler_structure_element_get_actual_text:
 * @poppler_structure_element: A #PopplerStructureElement
 *
 * Obtains the actual text enclosed by the element (and its child elements).
 * The actual text is mostly used for non-text elements like images and
 * figures which <em>do</em> have the graphical appearance of text, like
 * a logo. For those the actual text is the equivalent text to those
 * graphical elements which look like text when rendered.
 *
 * Note that for elements containing proper text, the function
 * poppler_structure_element_get_text() must be used instead.
 *
 * Return value: (transfer none): The actual text for the element, or %NULL
 *    if not defined.
 */
const gchar*
poppler_structure_element_get_actual_text (PopplerStructureElement *poppler_structure_element)
{
  g_return_val_if_fail (POPPLER_IS_STRUCTURE_ELEMENT (poppler_structure_element), NULL);
  g_assert (poppler_structure_element->elem);

  if (!poppler_structure_element->actual_text && poppler_structure_element->elem->getActualText ())
    {
      GooString *s = poppler_structure_element->elem->getActualText ();
      if (s)
        poppler_structure_element->actual_text = _poppler_goo_string_to_utf8 (s);
      delete s;
    }

  return poppler_structure_element->actual_text;
}

/**
 * poppler_structure_element_get_text:
 * @poppler_structure_element: A #PopplerStructureElement
 * @recursive: If %TRUE, the text of child elements is gathered recursively
 *   in logical order and returned as part of the result.
 *
 * Obtains the text enclosed by an element, or the subtree under an element.
 *
 * Return value: (transfer none): A string.
 */
const gchar*
poppler_structure_element_get_text (PopplerStructureElement *poppler_structure_element,
                                    gboolean                 recursive)
{
  if (recursive)
    {
      if (!poppler_structure_element->text_r)
        {
          GooString *s = poppler_structure_element->elem->getText (NULL, gTrue);
          if (s)
            poppler_structure_element->text_r = _poppler_goo_string_to_utf8 (s);
          delete s;
        }
      return poppler_structure_element->text_r;
    }

  if (!poppler_structure_element->text)
    {
      GooString *s = poppler_structure_element->elem->getText (NULL, gFalse);
      if (s)
        poppler_structure_element->text = _poppler_goo_string_to_utf8 (s);
      delete s;
    }
  return poppler_structure_element->text;
}


class SpanBuilder {
public:
  SpanBuilder():
    font(), text(), link(),
    map(globalParams->getTextEncoding()),
    glist(NULL),
    flags(0),
    color(0)
  {}

  ~SpanBuilder() {
    map->decRefCnt();
    g_list_free_full (glist, _poppler_text_span_free);
  }

  void process(const MCOpArray& ops) {
    for (MCOpArray::const_iterator i = ops.begin(); i != ops.end(); ++i)
      process(*i);
  }

  void process(const MCOp& op) {
    if (op.type == MCOp::Unichar) {
      int n = map->mapUnicode(op.unichar, buf, sizeof(buf));
      text.append(buf, n);
      return;
    }

    Guint oldFlags = flags;

    if (op.type == MCOp::Flags) {
      if (op.flags & MCOp::FlagFontBold)
        flags |= POPPLER_TEXT_SPAN_BOLD;
      else
        flags &= ~POPPLER_TEXT_SPAN_BOLD;

      if (op.flags & MCOp::FlagFontFixed)
        flags |= POPPLER_TEXT_SPAN_FIXED_WIDTH;
      else
        flags &= ~POPPLER_TEXT_SPAN_FIXED_WIDTH;

      if (op.flags & MCOp::FlagFontItalic)
        flags |= POPPLER_TEXT_SPAN_ITALIC;
      else
        flags &= ~POPPLER_TEXT_SPAN_ITALIC;
    }

    if (op.type == MCOp::Color && (color = op.color.rgbPixel ())) {
      flags |= POPPLER_TEXT_SPAN_COLOR;
    } else {
      flags &= ~POPPLER_TEXT_SPAN_COLOR;
    }

    if (op.type == MCOp::FontName) {
      if (op.value) {
        flags |= POPPLER_TEXT_SPAN_FONT;
        font.append(op.value);
      } else {
        flags &= ~POPPLER_TEXT_SPAN_FONT;
      }
    }

    if (flags != oldFlags)
      newSpan();
  }

  void newSpan() {
    // If there is no text, do not append a new PopplerTextSpan
    // and keep the attributes/flags for the next span.
    if (text.getLength ()) {
      PopplerTextSpan *span = g_slice_new0 (PopplerTextSpan);
      span->color = color;
      span->flags = flags;
      span->text = _poppler_goo_string_to_utf8 (&text);
      text.clear();

      if (font.getLength()) {
        span->font_name = _poppler_goo_string_to_utf8 (&font);
        font.clear();
      }

      if (link.getLength()) {
        assert(flags & POPPLER_TEXT_SPAN_LINK);
        span->link_target = _poppler_goo_string_to_utf8 (&link);
      }

      glist = g_list_append (glist, span);
    }

    // Link is always cleared
    link.clear();
  }

  GList* end() {
    GList *result = glist;
    glist = NULL;
    return result;
  }

private:
  GooString font;
  GooString text;
  GooString link;
  UnicodeMap *map;
  GList *glist;
  char buf[8];
  Guint flags;
  Guint color;
};


/**
 * poppler_structure_element_get_text_spans:
 * @poppler_structure_element: A #PopplerStructureElement
 *
 * Obtains the text enclosed by an element, as a #GList of #PopplerTextSpan
 * structures. Each item in the list is a piece of text which share the same
 * attributes, plus its attributes.
 *
 * Return value: (transfer none) (element-type PopplerTextSpan): A #GList
 *    of #PopplerTextSpan structures.
 */
GList*
poppler_structure_element_get_text_spans (PopplerStructureElement *poppler_structure_element)
{
  g_return_val_if_fail (POPPLER_IS_STRUCTURE_ELEMENT (poppler_structure_element), NULL);
  g_assert (poppler_structure_element->elem);

  if (!poppler_structure_element->elem->isContent ())
    return NULL;

  if (!poppler_structure_element->text_spans)
    {
      SpanBuilder builder;
      builder.process(poppler_structure_element->elem->getMCOps ());
      poppler_structure_element->text_spans = builder.end();
    }
  return poppler_structure_element->text_spans;
}

/**
 * poppler_text_span_is_fixed_width:
 * @poppler_text_span: a #PopplerTextSpan
 */
gboolean
poppler_text_span_is_fixed_width (PopplerTextSpan *poppler_text_span)
{
  return (poppler_text_span->flags & POPPLER_TEXT_SPAN_FIXED_WIDTH);
}

/**
 * poppler_text_span_is_serif_font:
 * @poppler_text_span: a #PopplerTextSpan
 */
gboolean
poppler_text_span_is_serif_font (PopplerTextSpan *poppler_text_span)
{
  return (poppler_text_span->flags & POPPLER_TEXT_SPAN_SERIF_FONT);
}

/**
 * poppler_text_span_is_bols:
 * @poppler_text_span: a #PopplerTextSpan
 */
gboolean
poppler_text_span_is_bold (PopplerTextSpan *poppler_text_span)
{
  return (poppler_text_span->flags & POPPLER_TEXT_SPAN_BOLD);
}

/**
 * poppler_text_span_is_link:
 * @poppler_text_span: a #PopplerTextSpan
 */
gboolean
poppler_text_span_is_link (PopplerTextSpan *poppler_text_span)
{
  return (poppler_text_span->flags & POPPLER_TEXT_SPAN_LINK);
}
