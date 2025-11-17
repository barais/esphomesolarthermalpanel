import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome import pins

from esphome.const import (
    CONF_ID,
    UNIT_CELSIUS,
    ICON_THERMOMETER,
    DEVICE_CLASS_TEMPERATURE,
    CONF_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
) 

CONF_DL_PIN = "dl_pin"
CONF_MODEL = "model"



# Namespace für die External Component
sensordlbus_ns = cg.esphome_ns.namespace("sensordlbus")
SensorDLBus = sensordlbus_ns.class_("SensorDLBus", cg.PollingComponent)

# **Hier wird der neue Key für device_type gesetzt**
CONF_DEVICE_TYPE = "devicetype"

