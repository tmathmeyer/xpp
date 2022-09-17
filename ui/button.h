#pragma once

#include "component.h"

namespace xpp::ui {

class XButton : public XComponent {
 public:
  XButton(std::string text);
  void Paint(xpp::ui::Graphics* g) override;
  std::optional<gfx::Rect> GetPreferredSize() const override;

  void Enter();
  void Exit();
  void Press();
  void Release();
 
 private:
  std::string content_;

  bool depressed_ = false;
  bool hovered_ = false;
};

}  // namespace xpp::ui