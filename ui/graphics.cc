#include "graphics.h"

#include "../xlib/xpixmap.h"
#include "canvas.h"

namespace xpp::ui {

Graphics::Graphics(std::shared_ptr<xlib::XGraphics> g,
                   std::shared_ptr<xlib::XColorMap> colormap,
                   std::shared_ptr<LookAndFeel> laf,
                   std::shared_ptr<xlib::XWindow> window,
                   uint32_t depth,
                   gfx::Rect size)
    : graphics_(std::move(g)),
      colormap_(std::move(colormap)),
      laf_(std::move(laf)),
      window_(window),
      depth_(depth),
      size_(size),
      offset_({0, 0}) {
  fonts_ = std::make_shared<LookAndFeel::FontCache>(graphics_);
  SetFont(
      laf_->AllocateFont(graphics_, "Fantasque Sans Mono", 10, fonts_.get()));
}

Graphics::Graphics(std::shared_ptr<xlib::XGraphics> g,
                   std::shared_ptr<xlib::XColorMap> colormap,
                   std::shared_ptr<LookAndFeel> laf,
                   std::shared_ptr<xlib::XWindow> window,
                   uint32_t depth,
                   gfx::Rect size,
                   gfx::Coord offset,
                   std::shared_ptr<LookAndFeel::FontCache> fonts)
    : graphics_(std::move(g)),
      colormap_(std::move(colormap)),
      laf_(std::move(laf)),
      window_(window),
      fonts_(fonts),
      depth_(depth),
      size_(size),
      offset_(offset) {
  SetFont(
      laf_->AllocateFont(graphics_, "Fantasque Sans Mono", 10, fonts_.get()));
}

void Graphics::SetColor(gfx::Color color) {
  color_ = color;
  graphics_->XSetForeground(laf_->GetXColor(colormap_, color).pixel);
}

void Graphics::SetColor(std::string color) {
  SetColor(laf_->GetColorByName(color));
}

void Graphics::SetFont(gfx::Font font) {
  font_ = font;
}

void Graphics::SetFont(std::string fontname) {
  font_ = laf_->GetFont(graphics_, fontname, font_, fonts_.get());
}

void Graphics::SetFontSize(uint16_t fontsize) {
  font_ = laf_->GetFont(graphics_, fontsize, font_, fonts_.get());
}

gfx::Rect Graphics::GetDimensions() const {
  return size_;
}

uint32_t Graphics::GetFontHeight() const {
  return font_.Height();
}

std::unique_ptr<XCanvas> Graphics::CreateCanvas(gfx::Rect size) const {
  std::shared_ptr<xlib::XPixmap> pixmap =
      window_->XCreatePixmap(size.width, size.height, depth_);
  Graphics graphics(pixmap->XCreateGC(colormap_), colormap_, laf_, window_,
                    depth_, size);
  return std::make_unique<XCanvas>(std::move(pixmap), std::move(graphics));
}

void Graphics::FillRect(gfx::Coord at, gfx::Rect size) {
  // TODO: use clamping utils of some sort
  graphics_->XFillRectangle(offset_.x + at.x, offset_.y + at.y, size.width,
                            size.height);
}

void Graphics::DrawRect(gfx::Coord at, gfx::Rect size) {
  // TODO: use clamping utils of some sort
  graphics_->XDrawRectangle(offset_.x + at.x, offset_.y + at.y, size.width,
                            size.height);
}

void Graphics::DrawRoundedRect(gfx::Coord at, gfx::Rect size, uint32_t radius) {
  uint32_t x = at.x + offset_.x;
  uint32_t y = at.y + offset_.y;
  uint32_t w = size.width;
  uint32_t h = size.height;
  graphics_->XSetLineAttributes(1, 0, 0, 0);
  graphics_->XDrawLine(x + radius, y, x + w - radius, y);
  graphics_->XDrawLine(x + radius, y + h - 1, x + w - radius, y + h - 1);
  graphics_->XDrawLine(x + w, y + radius, x + w, y + h - radius);
  graphics_->XDrawLine(x, y + radius, x, y + h - radius);

  graphics_->XDrawArc(x, y, radius * 2, radius * 2, 180 * 64, -90 * 64);
  graphics_->XDrawArc(x + w - radius * 2, y, radius * 2, radius * 2, 0,
                      90 * 64);
  graphics_->XDrawArc(x + w - radius * 2, y + h - radius * 2 - 1, radius * 2,
                      radius * 2, 0, -90 * 64);
  graphics_->XDrawArc(x, y + h - radius * 2 - 1, radius * 2, radius * 2,
                      180 * 64, 90 * 64);
}

void Graphics::FillRoundedRect(gfx::Coord at, gfx::Rect size, uint32_t radius) {
  uint32_t x = at.x + offset_.x;
  uint32_t y = at.y + offset_.y;
  uint32_t w = size.width;
  uint32_t h = size.height;

  graphics_->XFillRectangle(x + radius, y, w - radius * 2, h);
  graphics_->XFillRectangle(x, y + radius, w, h - radius * 2);
  graphics_->XFillArc(x, y, radius * 2, radius * 2, 180 * 64, -90 * 64);
  graphics_->XFillArc(x + w - radius * 2, y, radius * 2, radius * 2, 0,
                      90 * 64);
  graphics_->XFillArc(x + w - radius * 2, y + h - radius * 2 - 1, radius * 2,
                      radius * 2, 0, -90 * 64);
  graphics_->XFillArc(x, y + h - radius * 2 - 1, radius * 2, radius * 2,
                      180 * 64, 90 * 64);
}

void Graphics::CopyArea(std::shared_ptr<xlib::XDrawable> d, gfx::Coord at) {
  graphics_->XCopyArea(d->Drawable(), at.x, at.y, size_.width, size_.height,
                       offset_.x, offset_.y);
}

void Graphics::DrawText(gfx::Coord at, std::string message) {
  auto x = at.x + offset_.x;
  auto y = at.y + offset_.y;
  switch (font_.mode_) {
    case gfx::Font::TextRenderingMode::kXorg: {
      graphics_->XDrawString(x, y + font_.Height(), message.c_str(),
                             message.length());
      return;
    }
    case gfx::Font::TextRenderingMode::kXFT: {
      auto xft_color = laf_->GetXFTColor(graphics_, color_);
      XftDrawStringUtf8(
          fonts_->xft_ctx, &xft_color, font_.xft_font_, x, y + font_.Height(),
          (const unsigned char*)message.c_str(), message.length());
      return;
    }
    default: {
      puts("No renderer");
      return;
    }
  }
}

Graphics Graphics::SubGraphics(gfx::Coord offset, gfx::Rect size) {
  CHECK(offset.x >= 0);
  CHECK(offset.y >= 0);

  gfx::Coord new_offset = offset_ + offset;

  if (new_offset.x > offset_.x + size_.width)
    new_offset = {offset_.x + size_.width, new_offset.y};
  if (new_offset.y > offset_.y + size_.height)
    new_offset = {new_offset.x, new_offset.y + size_.height};

  uint32_t max_width = size_.width - offset.x;
  uint32_t max_height = size_.height - offset.y;
  gfx::Rect new_size = {
      std::min(size.width, max_width),
      std::min(size.height, max_height),
  };

  return {graphics_, colormap_, laf_,       window_,
          depth_,    new_size,  new_offset, fonts_};
}

}  // namespace xpp::ui