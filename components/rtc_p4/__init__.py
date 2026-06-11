import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome import automation

CODEOWNERS = ["@tiimsvk"]
AUTO_LOAD = ["time"]

rtc_p4_ns = cg.esphome_ns.namespace("rtc_p4")
RTCP4Component = rtc_p4_ns.class_("RTCP4Component", cg.Component)

# Actions
ReadTimeAction = rtc_p4_ns.class_("ReadTimeAction", automation.Action)
WriteTimeAction = rtc_p4_ns.class_("WriteTimeAction", automation.Action)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(RTCP4Component),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)


# Action schemas - pridaný synchronous parameter
CONF_RTC_TIME_ID = "rtc_time_id"

@automation.register_action(
    "rtc_p4.read_time",
    ReadTimeAction,
    cv.Schema({
        cv.GenerateID(CONF_RTC_TIME_ID): cv.use_id(rtc_p4_ns.class_("RTCP4Time")),
    }),
    synchronous=True
)
async def rtc_read_time_to_code(config, action_id, template_arg, args):
    parent = await cg.get_variable(config[CONF_RTC_TIME_ID])
    return cg.new_Pvariable(action_id, template_arg, parent)


@automation.register_action(
    "rtc_p4.write_time",
    WriteTimeAction,
    cv.Schema({
        cv.GenerateID(CONF_RTC_TIME_ID): cv.use_id(rtc_p4_ns.class_("RTCP4Time")),
    }),
    synchronous=True
)
async def rtc_write_time_to_code(config, action_id, template_arg, args):
    parent = await cg.get_variable(config[CONF_RTC_TIME_ID])
    return cg.new_Pvariable(action_id, template_arg, parent)
