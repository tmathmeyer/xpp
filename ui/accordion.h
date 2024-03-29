#pragma once

#include "component.h"
#include "panel.h"

namespace xpp::ui {

class XAccordion : public XPanel {
 public:
  enum class ComponentUsage : int32_t {
    kTitle = 1,
    kBody = 2,
  };

  XAccordion(std::string text);
  XAccordion(std::string, uint32_t);
  std::optional<uint32_t> GetPreferredHeight() override;
  std::optional<uint32_t> GetPreferredWidth() override;

  void AddComponent(std::unique_ptr<XComponent> component,
                    int32_t key) override;
  void AddComponent(std::unique_ptr<XComponent> component) override;
  void RemoveComponent(XComponent*) override;
  void RemoveAll() override;
  void SetLayout(std::unique_ptr<Layout>) override;
  void AddComponentListener(std::shared_ptr<ContainerListener>) override;

  void ToggleState();
  bool IsOpen();

 private:
  XPanel* container_ = nullptr;
  XComponent* title_ = nullptr;
  bool open_ = false;
};

}  // namespace xpp::ui