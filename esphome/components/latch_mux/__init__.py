from esphome import pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    CONF_CLOCK_PIN,
    CONF_DATA_PIN,
    CONF_ID,
    CONF_INVERTED,
    CONF_NUMBER,
    CONF_OE_PIN,
    CONF_OUTPUT,
    CONF_TYPE,
    CONF_COUNT,
)

MULTI_CONF = True

CODEOWNERS = ["@schulzh"]
DEPENDENCIES = []

latch_mux_ns = cg.esphome_ns.namespace("latch_mux")
LatchMuxComponent = latch_mux_ns.class_("LatchMuxComponent", cg.Component)
LatchMuxPin = latch_mux_ns.class_(
    "LatchMuxPin", cg.GPIOPin, cg.Parented.template(LatchMuxComponent)
)

CONF_LATCHMUX = "latch_mux"
CONF_DATA_PINS = "data_pins"
CONF_CS_PINS = "chipselect_pins"

TYPE_GPIO = "gpio"
TYPE_SPI = "spi"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.declare_id(LatchMuxComponent),
            cv.Required(CONF_DATA_PINS): cv.All(
                [pins.gpio_output_pin_schema],
                cv.Length(min=1),
            ),
            cv.Required(CONF_CS_PINS): cv.All(
                [pins.gpio_output_pin_schema],
                cv.Length(min=1),
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    data_pins = []
    for p in config[CONF_DATA_PINS]:
        pin = await cg.gpio_pin_expression(p)
        data_pins.append(pin)
    cg.add(var.set_data_pins(data_pins))

    cs_pins = []
    for p in config[CONF_CS_PINS]:
        pin = await cg.gpio_pin_expression(p)
        cs_pins.append(pin)
    cg.add(var.set_cs_pins(cs_pins))

def _validate_output_mode(value):
    if value.get(CONF_OUTPUT) is not True:
        raise cv.Invalid("Only output mode is supported")
    return value


LATCH_MUX_PIN_SCHEMA = pins.gpio_base_schema(
    LatchMuxPin,
    cv.int_range(min=0, max=2047),
    modes=[CONF_OUTPUT],
    mode_validator=_validate_output_mode,
    invertable=True,
).extend(
    {
        cv.Required(CONF_LATCHMUX): cv.use_id(LatchMuxComponent),
    }
)


def latch_mux_pin_final_validate(pin_config, parent_config):
    max_pins = len(parent_config[CONF_DATA_PINS]) * len(parent_config[CONF_CS_PINS])
    if pin_config[CONF_NUMBER] >= max_pins:
        raise cv.Invalid(f"Pin number must be less than {max_pins}")


@pins.PIN_SCHEMA_REGISTRY.register(
    CONF_LATCHMUX, LATCH_MUX_PIN_SCHEMA, latch_mux_pin_final_validate
)

async def latch_mux_pin_to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_parented(var, config[CONF_LATCHMUX])

    cg.add(var.set_pin(config[CONF_NUMBER]))
    cg.add(var.set_inverted(config[CONF_INVERTED]))
    return var