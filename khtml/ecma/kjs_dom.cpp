/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <kjs/operations.h>
#include <dom_string.h>
#include <dom_xml.h>

#include "kjs_text.h"
#include "kjs_dom.h"
#include "kjs_html.h"

using namespace KJS;

const TypeInfo DOMNode::info = { "Node", HostType, 0, 0, 0 };

KJSO DOMNode::get(const UString &p) const
{
  KJSO result;

  if (p == "nodeName")
    result = String(node.nodeName());
  else if (p == "nodeValue")
    result = String(node.nodeValue());
  else if (p == "nodeType")
    result = Number((unsigned long)node.nodeType());
  else if (p == "parentNode")
    result = DOMNode::getDOMNode(node.parentNode());
  else if (p == "childNodes")
    result = new DOMNodeList(node.childNodes());
  else if (p == "firstChild")
    result = DOMNode::getDOMNode(node.firstChild());
  else if (p == "lastChild")
    result = DOMNode::getDOMNode(node.lastChild());
  else if (p == "previousSibling")
    result = DOMNode::getDOMNode(node.previousSibling());
  else if (p == "nextSibling")
    result = DOMNode::getDOMNode(node.nextSibling());
  else if (p == "attributes")
    result = new DOMNamedNodeMap(node.attributes());
//  else if (p == "namespaceURI") // new for DOM2 - not yet in khtml
//    result = String(node.namespaceURI());
//  else if (p == "prefix") // new for DOM2 - not yet in khtml
//    result = String(node.prefix());
//  else if (p == "localName") // new for DOM2 - not yet in khtml
//    result = String(node.localName());
  else if (p == "ownerDocument")
    result = new DOMDocument(node.ownerDocument());
  // methods
  else if (p == "insertBefore")
    result = new DOMNodeFunc(node, DOMNodeFunc::InsertBefore);
  else if (p == "replaceChild")
    result = new DOMNodeFunc(node, DOMNodeFunc::ReplaceChild);
  else if (p == "removeChild")
    result = new DOMNodeFunc(node, DOMNodeFunc::RemoveChild);
  else if (p == "appendChild")
    result = new DOMNodeFunc(node, DOMNodeFunc::AppendChild);
  else if (p == "hasChildNodes")
    result = new DOMNodeFunc(node, DOMNodeFunc::HasChildNodes);
  else if (p == "cloneNode")
    result = new DOMNodeFunc(node, DOMNodeFunc::CloneNode);
//  else if (p == "normalize") // new for DOM2 - not yet in khtml
//    result = new DOMNodeFunc(node, DOMNodeFunc::Normalize);
//  else if (p == "supports") // new for DOM2 - not yet in khtml
//    result = new DOMNodeFunc(node, DOMNodeFunc::Supports);
  else
    result = Undefined();

  return result;
}

void DOMNode::put(const UString &p, const KJSO& v)
{
  if (p == "nodeValue") {
    node.setNodeValue(v.toString().value().string());
  }
//  else if (p == "prefix") { // new for DOM2 - not yet in khtml
//    node.setPrefix(v.toString().value().string());
//  }
}

KJSO DOMNode::getDOMNode(DOM::Node n)
{
  unsigned short nodeType = n.nodeType();
  if (nodeType == DOM::Node::ELEMENT_NODE) {
    DOM::Element element; // can't use static_cast here because of constructor :(
    element = n;
    if (element.isHTMLElement())
      return new HTMLElement(n);
    else
      return new DOMElement(element);
  }
  else if (nodeType == DOM::Node::TEXT_NODE)
    return new DOMText(n);
  else if (nodeType == DOM::Node::COMMENT_NODE)
    return new DOMCharacterData(n);
  else {
    return new DOMNode(n);
  }
}


Completion DOMNodeFunc::execute(const List &args)
{
  KJSO result;

  if (id == HasChildNodes)
    result = Boolean(node.hasChildNodes());
  else if (id == CloneNode) {
    Boolean b = args[0].toBoolean();
    result = DOMNode::getDOMNode(node.cloneNode(b.value()));
  } else {
    DOM::Node n1 = toNode(args[0]);
    if (id == AppendChild) {
      result = DOMNode::getDOMNode(node.appendChild(n1));
    } else if (id == RemoveChild) {
      result = DOMNode::getDOMNode(node.removeChild(n1));
    } else {
      DOM::Node n2 = toNode(args[1]);
      if (id == InsertBefore)
	result = DOMNode::getDOMNode(node.insertBefore(n1, n2));
      else
	result = DOMNode::getDOMNode(node.replaceChild(n1, n2));
    }
  }
  return Completion(Normal, result);
}

const TypeInfo DOMNodeList::info = { "DOMNodeList", HostType, 0, 0, 0 };

KJSO DOMNodeList::get(const UString &p) const
{
  KJSO result;

  if (p == "length")
    result = Number(list.length());
  // ### add support for list[index] syntax
  else if (p == "item")
    result = new DOMNodeListFunc(list, DOMNodeListFunc::Item);
  else
    result = Undefined();

  return result;
}

Completion DOMNodeListFunc::execute(const List &args)
{
  KJSO result;

  if (id == Item)
    result = DOMNode::getDOMNode(list.item(args[0].toNumber().intValue()));
  return Completion(Normal, result);
}

const TypeInfo DOMAttr::info = { "Attr", HostType, &DOMNode::info, 0, 0 };

KJSO DOMAttr::get(const UString &p) const
{
  KJSO result;
  if (p == "name") {
    result = String(attr.name()); }
  else if (p == "specified")
    result = Boolean(attr.specified());
  else if (p == "value")
    result = String(attr.value());
//  else if (p == "ownerElement") // new for DOM2 - not yet in khtml
//    rseult = DOMNode::getDOMNode(attr.ownerElement());
  else {
    KJSO tmp(new DOMNode(attr));
    result = tmp.get(p);
  }
  return result;
}

void DOMAttr::put(const UString &p, const KJSO& v)
{
  if (p == "value") {
    String s = v.toString();
    attr.setValue(s.value().string());
  } else {
    KJSO tmp(new DOMNode(attr));
    tmp.put(p, v);
  }
}

const TypeInfo DOMDocument::info = { "Document", HostType,
				     &DOMNode::info, 0, 0 };

KJSO DOMDocument::get(const UString &p) const
{
  KJSO result;

  if (p == "doctype")
    return new DOMDocumentType(doc.doctype());
  if (p == "implementation")
    return new DOMDOMImplementation(doc.implementation());
  else if (p == "documentElement")
    return new DOMElement(doc.documentElement());
  // methods
  else if (p == "createElement")
    return new DOMDocFunction(doc, DOMDocFunction::CreateElement);
  else if (p == "createDocumentFragment")
    return new DOMDocFunction(doc, DOMDocFunction::CreateDocumentFragment);
  else if (p == "createTextNode")
    return new DOMDocFunction(doc, DOMDocFunction::CreateTextNode);
  else if (p == "createComment")
    return new DOMDocFunction(doc, DOMDocFunction::CreateComment);
  else if (p == "createCDATASection")
    return new DOMDocFunction(doc, DOMDocFunction::CreateCDATASection);
  else if (p == "createProcessingInstruction")
    return new DOMDocFunction(doc, DOMDocFunction::CreateProcessingInstruction);
  else if (p == "createAttribute")
    return new DOMDocFunction(doc, DOMDocFunction::CreateAttribute);
  else if (p == "createEntityReference")
    return new DOMDocFunction(doc, DOMDocFunction::CreateEntityReference);
  else if (p == "getElementsByTagName")
    return new DOMDocFunction(doc, DOMDocFunction::GetElementsByTagName);
/*  else if (p == "importNode") // new for DOM2 - not yet in khtml
    return new DOMDocFunction(doc, DOMDocFunction::ImportNode);
  else if (p == "createElementNS") // new for DOM2 - not yet in khtml
    return new DOMDocFunction(doc, DOMDocFunction::CreateElementNS);
  else if (p == "createAttributeNS") // new for DOM2 - not yet in khtml
    return new DOMDocFunction(doc, DOMDocFunction::CreateAttributeNS);
  else if (p == "getElementsByTagNameNS") // new for DOM2 - not yet in khtml
    return new DOMDocFunction(doc, DOMDocFunction::GetElementsByTagNameNS);
  else if (p == "getElementById") // new for DOM2 - not yet in khtml
    return new DOMDocFunction(doc, DOMDocFunction::GetElementById);*/
  // look in base class (Document)
  KJSO tmp(new DOMNode(doc));
  result = tmp.get(p);

  return result;
}

DOMDocFunction::DOMDocFunction(DOM::Document d, int i)
  : doc(d), id(i)
{
}

#include <html_document.h>

Completion DOMDocFunction::execute(const List &args)
{
  KJSO result;
  String str = args[0].toString();
  DOM::DOMString s = str.value().string();

  DOM::Element e;

  DOM::HTMLDocument d = DOM::HTMLDocument();
  DOM::Element e2 = d.createElement(DOM::DOMString("br"));

  switch(id) {
  case CreateElement:
    e = doc.createElement(s);
    result = new DOMElement(doc.createElement(s));
    break;
  case CreateDocumentFragment:
    result = DOMNode::getDOMNode(doc.createDocumentFragment());
    break;
  case CreateTextNode:
    result = new DOMText(doc.createTextNode(s));
    break;
  case CreateComment:
    result = DOMNode::getDOMNode(doc.createComment(s));
    break;
  case CreateCDATASection:
    result = new DOMText(doc.createCDATASection(s));  /* TODO: okay ? */
    break;
  case CreateProcessingInstruction:
    result = DOMNode::getDOMNode(doc.createProcessingInstruction(args[0].toString().value().string(),
                                                                 args[1].toString().value().string()));
    break;
  case CreateAttribute:
    result = new DOMAttr(doc.createAttribute(s));
    break;
  case CreateEntityReference:
    result = DOMNode::getDOMNode(doc.createEntityReference(args[0].toString().value().string()));
    break;
  case GetElementsByTagName:
    result = new DOMNodeList(doc.getElementsByTagName(s));
    break;
    /* TODO */
/*  case ImportNode: // new for DOM2 - not yet in khtml
  case CreateElementNS: // new for DOM2 - not yet in khtml
  case CreateAttributeNS: // new for DOM2 - not yet in khtml
  case GetElementsByTagNameNS: // new for DOM2 - not yet in khtml
  case GetElementById: // new for DOM2 - not yet in khtml*/
  default:
    result = Undefined();
  }

  return Completion(Normal, result);
}

const TypeInfo DOMElement::info = { "Element", HostType,
				    &DOMNode::info, 0, 0 };

KJSO DOMElement::get(const UString &p) const
{
  KJSO result;

  if (p == "tagName")
    result = String(element.tagName());
  else if (p == "getAttribute")
    return new DOMElementFunction(element, DOMElementFunction::GetAttribute);
  else if (p == "setAttribute")
    return new DOMElementFunction(element, DOMElementFunction::SetAttribute);
  else if (p == "removeAttribute")
    return new DOMElementFunction(element, DOMElementFunction::RemoveAttribute);
  else if (p == "getAttributeNode")
    return new DOMElementFunction(element, DOMElementFunction::GetAttributeNode);
  else if (p == "setAttributeNode")
    return new DOMElementFunction(element, DOMElementFunction::SetAttributeNode);
  else if (p == "removeAttributeNode")
    return new DOMElementFunction(element, DOMElementFunction::RemoveAttributeNode);
  else if (p == "getElementsByTagName")
    return new DOMElementFunction(element, DOMElementFunction::GetElementsByTagName);
/*  else if (p == "getAttributeNS") // new for DOM2 - not yet in khtml
    return new DOMElementFunction(element, DOMElementFunction::GetAttributeNS);
  else if (p == "setAttributeNS") // new for DOM2 - not yet in khtml
    return new DOMElementFunction(element, DOMElementFunction::SetAttributeNS);
  else if (p == "removeAttributeNS") // new for DOM2 - not yet in khtml
    return new DOMElementFunction(element, DOMElementFunction::RemoveAttributeNS);
  else if (p == "getAttributeNodeNS") // new for DOM2 - not yet in khtml
    return new DOMElementFunction(element, DOMElementFunction::GetAttributeNodeNS);
  else if (p == "setAttributeNodeNS") // new for DOM2 - not yet in khtml
    return new DOMElementFunction(element, DOMElementFunction::SetAttributeNodeNS);
  else if (p == "getElementsByTagNameNS") // new for DOM2 - not yet in khtml
    return new DOMElementFunction(element, DOMElementFunction::GetElementsByTagNameNS);
  else if (p == "hasAttribute") // new for DOM2 - not yet in khtml
    return new DOMElementFunction(element, DOMElementFunction::HasAttribute);
  else if (p == "hasAttributeNS") // new for DOM2 - not yet in khtml
    return new DOMElementFunction(element, DOMElementFunction::HasAttributeNS);*/
  else {
    KJSO tmp(new DOMNode(element));
    return tmp.get(p);
  }

  return result;
}


DOMElementFunction::DOMElementFunction(DOM::Element e, int i)
  : element(e), id(i)
{
}

Completion DOMElementFunction::execute(const List &args)
{
  KJSO result;

  switch(id) {
    case GetAttribute:
      result = String(element.getAttribute(args[0].toString().value().string()));
      break;
    case SetAttribute:
      element.setAttribute(args[0].toString().value().string(),args[1].toString().value().string());
      result = Undefined();
      break;
    case RemoveAttribute:
      element.removeAttribute(args[0].toString().value().string());
      result = Undefined();
      break;
    case GetAttributeNode:
      result = DOMNode::getDOMNode(element.getAttributeNode(args[0].toString().value().string()));
      break;
    case SetAttributeNode:
      result = DOMNode::getDOMNode(element.setAttributeNode(DOMNode(KJS::toNode(args[0])).toNode()));
      break;
    case RemoveAttributeNode:
      result = DOMNode::getDOMNode(element.removeAttributeNode(DOMNode(KJS::toNode(args[0])).toNode()));
      break;
    case GetElementsByTagName:
      result = new DOMNodeList(element.getElementsByTagName(args[0].toString().value().string()));
      break;
/*    case GetAttributeNS: // new for DOM2 - not yet in khtml
    case SetAttributeNS: // new for DOM2 - not yet in khtml
    case RemoveAttributeNS: // new for DOM2 - not yet in khtml
    case GetAttributeNodeNS: // new for DOM2 - not yet in khtml
    case SetAttributeNodeNS: // new for DOM2 - not yet in khtml
    case GetElementsByTagNameNS: // new for DOM2 - not yet in khtml
    case HasAttribute: // new for DOM2 - not yet in khtml
    case HasAttributeNS: // new for DOM2 - not yet in khtml*/
  default:
    result = Undefined();
  }

  return Completion(Normal, result);
}

const TypeInfo DOMDOMImplementation::info = { "DOMImplementation", HostType, 0, 0, 0 };

KJSO DOMDOMImplementation::get(const UString &p) const
{
  if (p == "hasFeature")
    return new DOMDOMImplementationFunction(implementation, DOMDOMImplementationFunction::HasFeature);
//  else if (p == "createDocumentType") // new for DOM2 - not yet in khtml
//    return new DOMDOMImplementationFunction(implementation, DOMDOMImplementationFunction::CreateDocumentType);
//  else if (p == "createDocument") // new for DOM2 - not yet in khtml
//    return new DOMDOMImplementationFunction(implementation, DOMDOMImplementationFunction::CreateDocument);
  else
    return Undefined();
}

DOMDOMImplementationFunction::DOMDOMImplementationFunction(DOM::DOMImplementation impl, int i)
  : implementation(impl), id(i)
{
}

Completion DOMDOMImplementationFunction::execute(const List &args)
{
  KJSO result;

  switch(id) {
    case HasFeature:
      result = Boolean(implementation.hasFeature(args[0].toString().value().string(),args[1].toString().value().string()));
      break;
/*    case CreateDocumentType: // new for DOM2 - not yet in khtml
    case CreateDocument: // new for DOM2 - not yet in khtml*/
    default:
      result = Undefined();
  }

  return Completion(Normal, result);
}

const TypeInfo DOMDocumentType::info = { "DocumentType", HostType, &DOMNode::info, 0, 0 };

KJSO DOMDocumentType::get(const UString &p) const
{
  KJSO result;

  if (p == "name")
    result = String(type.name());
// ###  else if (p == "entities")
//    result = new DOMNamedNodeMap(type.entities());
// ###  else if (p == "notations")
//    result = new DOMNamedNodeMap(type.notations());
//  else if (p == "publicId") // new for DOM2 - not yet in khtml
//    result = String(type.publicId());
//  else if (p == "systemId") // new for DOM2 - not yet in khtml
//    result = String(type.systemId());
//  else if (p == "internalSubset") // new for DOM2 - not yet in khtml
//    result = String(type.internalSubset());
  else {
    KJSO tmp(new DOMNode(type));
    return tmp.get(p);
  }

  return result;
}

const TypeInfo DOMNamedNodeMap::info = { "NamedNodeMap", HostType, 0, 0, 0 };

KJSO DOMNamedNodeMap::get(const UString &p) const
{
  KJSO result;

  if (p == "length")
    return Number((unsigned long)map.length());
  else if (p == "getNamedItem")
    result = new DOMNamedNodeMapFunction(map, DOMNamedNodeMapFunction::GetNamedItem);
  else if (p == "setNamedItem")
    result = new DOMNamedNodeMapFunction(map, DOMNamedNodeMapFunction::SetNamedItem);
  else if (p == "removeNamedItem")
    result = new DOMNamedNodeMapFunction(map, DOMNamedNodeMapFunction::RemoveNamedItem);
  else if (p == "item")
    result = new DOMNamedNodeMapFunction(map, DOMNamedNodeMapFunction::Item);
//  else if (p == "getNamedItemNS") // new for DOM2 - not yet in khtml
//    result = new DOMNamedNodeMapFunction(map, DOMNamedNodeMapFunction::GetNamedItemNS);
//  else if (p == "setNamedItemNS") // new for DOM2 - not yet in khtml
//    result = new DOMNamedNodeMapFunction(map, DOMNamedNodeMapFunction::SetNamedItemNS);
//  else if (p == "removeNamedItemNS") // new for DOM2 - not yet in khtml
//    result = new DOMNamedNodeMapFunction(map, DOMNamedNodeMapFunction::RemoveNamedItemNS);
  else
    return Undefined();


  return result;
}

DOMNamedNodeMapFunction::DOMNamedNodeMapFunction(DOM::NamedNodeMap m, int i)
  : map(m), id(i)
{
}

Completion DOMNamedNodeMapFunction::execute(const List &args)
{
  KJSO result;

  switch(id) {
    case GetNamedItem:
      result = DOMNode::getDOMNode(map.getNamedItem(args[0].toString().value().string()));
      break;
    case SetNamedItem:
      result = DOMNode::getDOMNode(map.setNamedItem(DOMNode(KJS::toNode(args[0])).toNode()));
      break;
    case RemoveNamedItem:
      result = DOMNode::getDOMNode(map.removeNamedItem(args[0].toString().value().string()));
      break;
    case Item:
      // ### add support for list[index] syntax
      result = DOMNode::getDOMNode(map.item(args[0].toNumber().intValue()));
      break;
/*    case GetNamedItemNS: // new for DOM2 - not yet in khtml
    case SetNamedItemNS: // new for DOM2 - not yet in khtml
    case RemoveNamedItemNS: // new for DOM2 - not yet in khtml*/
    default:
      result = Undefined();
  }

  return Completion(Normal, result);
}

const TypeInfo DOMProcessingInstruction::info = { "ProcessingInstruction", HostType, &DOMNode::info, 0, 0 };

KJSO DOMProcessingInstruction::get(const UString &p) const
{
  KJSO result;
  if (p == "target")
    result = String(instruction.target());
  else if (p == "data")
    result = String(instruction.data());
  else {
    KJSO tmp(new DOMNode(instruction));
    result = tmp.get(p);
  }
  return result;
}

void DOMProcessingInstruction::put(const UString &p, const KJSO& v)
{
  if (p == "data")
    instruction.setData(v.toString().value().string());
  else {
    KJSO tmp(new DOMNode(instruction));
    tmp.put(p, v);
  }
}

const TypeInfo DOMNotation::info = { "Notation", HostType, &DOMNode::info, 0, 0 };

KJSO DOMNotation::get(const UString &p) const
{
  KJSO result;

  if (p == "publicId")
    result = String(notation.publicId());
  else if (p == "systemId")
    result = String(notation.systemId());
  else {
    KJSO tmp(new DOMNode(notation));
    return tmp.get(p);
  }

  return result;
}

const TypeInfo DOMEntity::info = { "Entity", HostType, &DOMNode::info, 0, 0 };

KJSO DOMEntity::get(const UString &p) const
{
  KJSO result;

  if (p == "publicId")
    result = String(entity.publicId());
  else if (p == "systemId")
    result = String(entity.systemId());
  else if (p == "notationName")
    result = String(entity.notationName());
  else {
    KJSO tmp(new DOMNode(entity));
    return tmp.get(p);
  }

  return result;
}


