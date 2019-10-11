#include <escher/text_input.h>
#include <ion/unicode/utf8_decoder.h>
#include <ion/unicode/utf8_helper.h>
#include <assert.h>

/* TextInput::ContentView */

static inline const char * minCharPointer(const char * x, const char * y) { return x < y ? x : y; }
static inline const char * maxCharPointer(const char * x, const char * y) { return x > y ? x : y; }

void TextInput::ContentView::setCursorLocation(const char * location) {
  assert(location != nullptr);
  assert(location >= editedText());
  const char * adjustedLocation = minCharPointer(location, editedText() + editedTextLength());
  m_cursorLocation = adjustedLocation;
  layoutSubviews();
}

void TextInput::ContentView::setFont(const KDFont * font) {
  m_font = font;
  markRectAsDirty(bounds());
}

KDRect TextInput::ContentView::cursorRect() {
  return glyphFrameAtPosition(editedText(), m_cursorLocation);
}

void TextInput::ContentView::addSelection(const char * left, const char * right) {
  bool emptySelection = selectionIsEmpty();
  if (emptySelection) {
    m_selectionStart = left;
    m_selectionEnd = right;
  } else if (left == m_selectionEnd) {
    m_selectionEnd = right;
  } else if (right == m_selectionStart) {
    m_selectionStart = left;
  } else if (right == m_selectionEnd) {
    if (left >= m_selectionStart) {
     m_selectionEnd = left;
    } else {
      m_selectionEnd = m_selectionStart;
      m_selectionStart = left;
    }
  } else {
    assert(left == m_selectionStart);
    if (right <= m_selectionEnd) {
      m_selectionStart = right;
    } else {
      m_selectionStart = m_selectionEnd;
      m_selectionEnd = right;
    }
  }
  reloadRectFromAndToPositions(left, right);
  if (m_selectionStart == m_selectionEnd) {
    m_selectionStart = nullptr;
    m_selectionEnd = nullptr;
  }
}

bool TextInput::ContentView::resetSelection() {
  const char * previousStart = m_selectionStart;
  if (previousStart == nullptr) {
    return false;
  }
  const char * previousEnd = m_selectionEnd;
  m_selectionStart = nullptr;
  m_selectionEnd = nullptr;
  reloadRectFromAndToPositions(previousStart, previousEnd);
  return true;
}

bool TextInput::ContentView::selectionIsEmpty() const {
  assert(m_selectionStart != nullptr || m_selectionEnd == nullptr);
  assert(m_selectionEnd != nullptr || m_selectionStart == nullptr);
  return m_selectionStart == nullptr;
}

void TextInput::ContentView::layoutSubviews(bool force) {
  m_cursorView.setFrame(cursorRect(), force);
}

void TextInput::ContentView::reloadRectFromPosition(const char * position, bool includeFollowingLines) {
  markRectAsDirty(dirtyRectFromPosition(position, includeFollowingLines));
}

void TextInput::ContentView::reloadRectFromAndToPositions(const char * start, const char * end) {
  KDRect startFrame = glyphFrameAtPosition(text(), start);
  KDRect endFrame = glyphFrameAtPosition(text(), end);
  bool onSameLine = startFrame.y() == endFrame.y();
  markRectAsDirty(KDRect(
        onSameLine ? startFrame.x() : 0,
        startFrame.y(),
        onSameLine ? endFrame.right() - startFrame.left() : bounds().width(),
        endFrame.bottom() - startFrame.top() + 1));
}

KDRect TextInput::ContentView::dirtyRectFromPosition(const char * position, bool includeFollowingLines) const {
  KDRect glyphRect = glyphFrameAtPosition(text(), position);
  if (!includeFollowingLines) {
    KDRect dirtyRect = KDRect(
        glyphRect.x(),
        glyphRect.y(),
        bounds().width() - glyphRect.x(),
        glyphRect.height());
    return dirtyRect;
  }
  KDRect dirtyRect = KDRect(
      0,
      glyphRect.y(),
      bounds().width(),
      bounds().height() - glyphRect.y());
  return dirtyRect;
}

/* TextInput */

bool TextInput::removePreviousGlyph() {
  contentView()->removePreviousGlyph();
  scrollToCursor();
  return true;
}

void TextInput::scrollToCursor() {
  /* Technically, we do not need to overscroll in text input. However, we should
   * layout the scroll view before calling scrollToContentRect (in case the size
   * of the scroll view has changed) and then call scrollToContentRect which
   * calls another layout of the scroll view if the offset has evolved.
   *
   * In order to avoid requiring two layouts, we allow overscrolling in
   * scrollToContentRect, and the last layout of the scroll view corrects the
   * size of the scroll view only once. */
  scrollToContentRect(contentView()->cursorRect(), true);
}

bool TextInput::setCursorLocation(const char * location) {
  assert(location != nullptr);
  const char * adjustedLocation = maxCharPointer(location, text());
  willSetCursorLocation(&adjustedLocation);
  contentView()->setCursorLocation(adjustedLocation);
  scrollToCursor();
  return true;
}

bool TextInput::insertTextAtLocation(const char * text, const char * location) {
  if (contentView()->insertTextAtLocation(text, location)) {
    /* We layout the scrollable view before scrolling to cursor because the
     * content size might have changed. */
    layoutSubviews();
    scrollToCursor();
    return true;
  }
  return false;
}

bool TextInput::removeEndOfLine() {
  if (privateRemoveEndOfLine()) {
    scrollToCursor();
    return true;
  }
  return false;
}

bool TextInput::moveCursorLeft() {
  if (cursorLocation() <= text()) {
    assert(cursorLocation() == text());
    return false;
  }
  UTF8Decoder decoder(text(), cursorLocation());
  return setCursorLocation(decoder.previousGlyphPosition());
}

bool TextInput::moveCursorRight() {
  if (UTF8Helper::CodePointIs(cursorLocation(), UCodePointNull)) {
    return false;
  }
  UTF8Decoder decoder(cursorLocation());
  return setCursorLocation(decoder.nextGlyphPosition());
}

bool TextInput::selectLeftRight(bool left) {
  const char * cursorLoc = cursorLocation();
  if ((left && cursorLoc <= text())
     || (!left && UTF8Helper::CodePointIs(cursorLoc, UCodePointNull)))
  {
    assert(!left || cursorLoc == text());
    return false;
  }
  UTF8Decoder decoder(text(), cursorLoc);
  const char * nextCursorLocation = left ? decoder.previousGlyphPosition() : decoder.nextGlyphPosition();
  contentView()->addSelection(left ? nextCursorLocation : cursorLoc, left ? cursorLoc : nextCursorLocation); //TODO LEA adjusted location?
  return setCursorLocation(nextCursorLocation);
}

bool TextInput::privateRemoveEndOfLine() {
  return contentView()->removeEndOfLine();
}
