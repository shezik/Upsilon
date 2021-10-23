#include "tex_parser.h"
#include <ion/unicode/utf8_decoder.h>

namespace Reader {

TexParser::TexParser(const char * text, const char * endOfText) : 
  m_text(text), 
  m_endOfText(endOfText),
  m_hasError(false)
{
    
}

Layout TexParser::getLayout() {
  Layout layout = popText(0);

  if (m_hasError) {
    return CodePointLayout::Builder(CodePoint(0xFFD));
  }

  return layout;
}

Layout TexParser::popBlock() {
  while (*m_text == ' ') {
    m_text ++;
  }

  if (*m_text == '{') {
    m_text ++;
    return popText('}');
  }

  if (*m_text == '\\') {
    m_text ++;
    return popCommand();
  }

  if (m_text >= m_endOfText) {
    m_hasError = true;
  }

  UTF8Decoder decoder(m_text);
  m_text ++;
  return CodePointLayout::Builder(decoder.nextCodePoint());
}

Layout TexParser::popText(char stop) {
  HorizontalLayout layout = HorizontalLayout::Builder();
  const char * start = m_text;
  
  while (m_text < m_endOfText && *m_text != stop) {
    switch (*m_text) {
      case '\\':
        if (start != m_text) {
          layout.addOrMergeChildAtIndex(LayoutHelper::String(start, m_text - start), layout.numberOfChildren(), false);
        }
        m_text ++;
        layout.addOrMergeChildAtIndex(popCommand(), layout.numberOfChildren(), false);
        start = m_text;
        break;
      case ' ':
        if (start != m_text) {
          layout.addOrMergeChildAtIndex(LayoutHelper::String(start, m_text - start), layout.numberOfChildren(), false);
        }
        m_text ++;
        start = m_text;
        break;
      case '^':
        if (start != m_text) {
          layout.addOrMergeChildAtIndex(LayoutHelper::String(start, m_text - start), layout.numberOfChildren(), false);
        }
        m_text ++;
        layout.addOrMergeChildAtIndex(VerticalOffsetLayout::Builder(popBlock(), VerticalOffsetLayoutNode::Position::Superscript), layout.numberOfChildren(), false);
        start = m_text;
        break;
      case '_':
        if (start != m_text) {
          layout.addOrMergeChildAtIndex(LayoutHelper::String(start, m_text - start), layout.numberOfChildren(), false);
        }
        m_text ++;
        layout.addOrMergeChildAtIndex(VerticalOffsetLayout::Builder(popBlock(), VerticalOffsetLayoutNode::Position::Subscript), layout.numberOfChildren(), false);
        start = m_text;
        break;
      default:
        m_text ++;
    }
  }

  if (start != m_text) {
    layout.addOrMergeChildAtIndex(LayoutHelper::String(start, m_text - start), layout.numberOfChildren(), false);
  }
  
  m_text ++;

  if (layout.numberOfChildren() == 1) {
    return layout.squashUnaryHierarchyInPlace();
  }

  return layout;
}

Layout TexParser::popCommand() {
  if (strncmp(k_fracCommand, m_text, strlen(k_fracCommand)) == 0) {
    m_text += strlen(k_fracCommand);
    if (*m_text == ' ' || *m_text == '{') {
      return popFracCommand();
    } 
  }
  else if (strncmp(k_sqrtCommand, m_text, strlen(k_sqrtCommand)) == 0) {
    m_text += strlen(k_sqrtCommand);
    if (*m_text == ' ' || *m_text == '{' || *m_text == '[') {
      return popSqrtCommand();
    }
  }
  else if (strncmp(k_thetaCommand, m_text, strlen(k_thetaCommand)) == 0) {
    m_text += strlen(k_thetaCommand);
    if (*m_text == ' ') {
      return popThetaCommand();
    }
  }
  else if (strncmp(k_piCommand, m_text, strlen(k_piCommand)) == 0) {
    m_text += strlen(k_piCommand);
    if (*m_text == ' ') {
      return popPiCommand();
    }
  }
  
  m_hasError = true;
  return LayoutHelper::String(m_text, strlen(m_text));
}

Layout TexParser::popFracCommand() {
  return FractionLayout::Builder(popBlock(), popBlock());
}

Layout TexParser::popSqrtCommand() {
  while (*m_text == ' ') {
    m_text ++;
  }
  m_text++;
  if (*m_text == '[') {
    return NthRootLayout::Builder(popText(']'), popBlock());
  }
  else {
    return NthRootLayout::Builder(popBlock());
  }
}

Layout TexParser::popThetaCommand() {
  return CodePointLayout::Builder(UCodePointGreekSmallLetterTheta);
}

Layout TexParser::popPiCommand() {
  return CodePointLayout::Builder(UCodePointGreekSmallLetterPi);
}

}