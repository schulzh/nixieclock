from esphome.components import number
import esphome.config_validation as cv
import esphome.codegen as cg
from .. import CONF_LATCHMUX, latch_mux_ns, LatchMuxComponent
from esphome.const import (
    CONF_ID,
    CONF_PINS,
)

DEPENDENCIES = ["latch_mux"]
CODEOWNERS = ["@schulzha"]

CONF_COUNT_BACK = "count_back"
CONF_COUNT_BACK_SPEED = "count_back_speed"

LatchMuxNumberComponent = latch_mux_ns.class_(
    "LatchMuxNumberComponent", number.Number, cg.PollingComponent
)

CONFIG_SCHEMA = cv.All(
    number.number_schema(LatchMuxNumberComponent)
    .extend(
        {
            cv.Required(CONF_LATCHMUX): cv.use_id(LatchMuxComponent),
            cv.Required(CONF_PINS): cv.All(
                [cv.int_range(min=0)],
                cv.Length(min=1, max=10),
            ),
            cv.Optional(CONF_COUNT_BACK, default=False): cv.boolean,
            cv.Optional(CONF_COUNT_BACK_SPEED, default=20): cv.int_range(
                min=1, max=100
            ),
        }
    )
    .extend(cv.polling_component_schema("60s")),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_parented(var, config[CONF_LATCHMUX])
    await cg.register_component(var, config)
    await number.register_number(
        var, config, min_value=0, max_value=len(config[CONF_PINS]) - 1, step=1
    )
    cg.add(var.set_pins(config[CONF_PINS]))
    cg.add(var.set_count_back(config[CONF_COUNT_BACK]))
    cg.add(var.set_count_back_speed(config[CONF_COUNT_BACK_SPEED]))