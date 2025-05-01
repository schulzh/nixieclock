#include "latch_mux.h"
#include "esphome/core/log.h"

namespace esphome {
namespace latch_mux {

static const char *const TAG = "latch_mux";

void LatchMuxComponent::pre_setup_() {
  ESP_LOGCONFIG(TAG, "Setting up Latch Mux...");
}
void LatchMuxComponent::post_setup_() {
  this->write_gpio();
}

void LatchMuxComponent::setup() {
  this->pre_setup_();
  for(auto *pin : this->data_pins_) {
    pin->setup();
    pin->digital_write(false);
  }
  for(auto *pin : this->cs_pins_) {
    pin->setup();
    pin->digital_write(false);
  }

  this->post_setup_();
}

void LatchMuxComponent::dump_config() { ESP_LOGCONFIG(TAG, "Latch Mux:"); }

void LatchMuxComponent::internal_write_(uint16_t pin, bool value) {
  ESP_LOGD(TAG, "Write %d to %u", value, pin);
  if (pin >= this->max_pins_) {
    ESP_LOGE(TAG, "Pin %u is out of range! Maximum pin number with %u latches on a %u wide bus is %u",
      pin, this->cs_pins_.size(), this->data_pins_.size(), this->max_pins_ - 1);
    return;
  }
  if (value) {
    this->output_bytes_[pin / 8] |= (1 << (pin % 8));
  } else {
    this->output_bytes_[pin / 8] &= ~(1 << (pin % 8));
  }
}

void LatchMuxComponent::digital_write_(uint16_t pin, bool value) {
  this->internal_write_(pin, value);
  this->write_gpio(); // this can be optimized by only writing to the required latch
}

void LatchMuxComponent::write_gpio() {
  for (uint8_t c = 0; c < this->cs_pins_.size(); c++) {
    for (uint8_t d = 0; d < this->data_pins_.size(); d++) {
      uint8_t bitIdx = c * data_pins_.size() + d;
      bool bit = this->output_bytes_[bitIdx / 8] & (1 << (bitIdx % 8));
      this->data_pins_[d]->digital_write(bit);
    }
    this->cs_pins_[c]->digital_write(true);
    this->cs_pins_[c]->digital_write(false);
  }
}

void LatchMuxPin::digital_write(bool value) {
  this->parent_->digital_write_(this->pin_, value != this->inverted_);
}
std::string LatchMuxPin::dump_summary() const { return str_snprintf("%u via Latch Mux", 18, pin_); }

}  // namespace latch_mux
}  // namespace esphome