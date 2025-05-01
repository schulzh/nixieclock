#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include <vector>

namespace esphome {
namespace latch_mux {

class LatchMuxComponent : public Component {
 public:
  LatchMuxComponent() = default;
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::IO; }
  void set_data_pins(std::vector<GPIOPin *> data_pins) {
    data_pins_ = std::move(data_pins);
    resize();
  };
  void set_cs_pins(std::vector<GPIOPin *> cs_pins) {
    cs_pins_ = std::move(cs_pins);
    resize();
  };

 protected:
  void resize() {
    max_pins_ = data_pins_.size() * cs_pins_.size();
    uint8_t bytes = (max_pins_ - 1) / 8 + 1;
    output_bytes_.resize(bytes);
  }

  friend class LatchMuxPin;
  friend class LatchMuxNumberComponent;
  void digital_write_(uint16_t pint, bool value);
  void internal_write_(uint16_t pint, bool value);
  void write_gpio();

  void pre_setup_();
  void post_setup_();

  std::vector<GPIOPin*> data_pins_;
  std::vector<GPIOPin*> cs_pins_;
  uint8_t max_pins_;
  std::vector<uint8_t> output_bytes_;
};

class LatchMuxPin : public GPIOPin, public Parented<LatchMuxComponent> {
 public:
  void setup() override {}
  void pin_mode(gpio::Flags flags) override {}
  bool digital_read() override { return false; }
  void digital_write(bool value) override;
  std::string dump_summary() const override;

  void set_pin(uint16_t pin) { pin_ = pin; }
  void set_inverted(bool inverted) { inverted_ = inverted; }

  /// Always returns `gpio::Flags::FLAG_OUTPUT`.
  gpio::Flags get_flags() const override { return gpio::Flags::FLAG_OUTPUT; }

 protected:
  uint16_t pin_;
  bool inverted_;
};

}  // namespace sn74hc595
}  // namespace esphome