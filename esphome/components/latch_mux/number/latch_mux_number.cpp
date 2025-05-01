#include "latch_mux_number.h"
#include "esphome/core/log.h"

namespace esphome {
  namespace latch_mux {

    static const char* const TAG = "latch_mux.number";

    void LatchMuxNumberComponent::setup() {
      ESP_LOGCONFIG(TAG, "Setting up nixie number...");
      // FIXME: set all pins to off
    }  // LatchMuxNumberComponent::setup()

    void LatchMuxNumberComponent::dump_config() {
      LOG_NUMBER("", "Nixie Number", this);
      ESP_LOGCONFIG(TAG, "  Count Back: %s", YESNO(this->count_back_));
      if (this->count_back_)
        ESP_LOGCONFIG(TAG, "  Count Back Speed: %d", this->count_back_speed_);
      ESP_LOGCONFIG(TAG, "  Output Pins:");
      for (auto pin = this->pins_.begin(); pin != this->pins_.end(); ++pin)
        ESP_LOGCONFIG(TAG, "    Digit %d: pin number %d", std::distance(this->pins_.begin(), pin), *pin);
      LOG_UPDATE_INTERVAL(this);
    }  // LatchMuxNumberComponent::dump_config()

    void LatchMuxNumberComponent::loop() {
      if (this->count_back_ && this->my_current_number_ > this->my_target_number_) {
        if (millis() > this->count_back_last_called_ + this->count_back_speed_) {
          this->count_back_last_called_ = millis();
          this->set_outputs_(--this->my_current_number_);
        }
      }
    }  // LatchMuxNumberComponent::loop()

    void LatchMuxNumberComponent::control(float value) {
      uint8_t new_value = static_cast<uint8_t>(value);
      if (this->my_target_number_ == new_value) return;
      this->my_target_number_ = new_value;

      if (!this->count_back_ || this->my_current_number_ < this->my_target_number_) {
        this->my_current_number_ = this->my_target_number_;
      }
      this->set_outputs_(this->my_current_number_);
      this->publish_state(this->my_target_number_);
    }  // LatchMuxNumberComponent::control(float value)

    void LatchMuxNumberComponent::update() { this->publish_state(this->my_target_number_); }

    void LatchMuxNumberComponent::set_outputs_(uint8_t value) {
      uint8_t pin_en = this->pins_[value];

      for (uint8_t pin : this->pins_) {
          this->parent_->internal_write_(pin, (pin == pin_en));
      }

      this->parent_->write_gpio();
    }  // LatchMuxNumberComponent::_set_outputs(uint8_t value)

  }  // namespace latch_mux
}  // namespace esphome