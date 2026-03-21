import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.core import CORE

CODEOWNERS = ["@trip5"]
DEPENDENCIES = ["wifi"]
MULTI_CONF = False

ehdp_ns = cg.esphome_ns.namespace("ehdp")
EhDPComponent = ehdp_ns.class_("EhDPComponent", cg.Component)

CONF_DEVICE_NAME    = "name"
CONF_PROJECT        = "project"
CONF_FIRMWARE       = "firmware"
CONF_MATERIAL_SYMBOL  = "material_symbol"
CONF_VERSION        = "version"
CONF_MDNS           = "mdns"
CONF_UI_PORT        = "ui_port"
CONF_WEBUI          = "webui"
CONF_CAPABILITIES   = "capabilities"


def validate_has_name(config):
    # If the user provides any of these, we're good.
    # If they are all missing, we will attempt to fall back to ESPHome global constants in to_code
    # We don't strictly *need* to enforce this in the schema if it's possible to derive it.
    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(EhDPComponent),
            cv.Optional(CONF_DEVICE_NAME): cv.string,
            cv.Optional(CONF_PROJECT):     cv.string,
            cv.Optional(CONF_FIRMWARE):    cv.string,
            cv.Optional(CONF_MATERIAL_SYMBOL): cv.string,  # Hex string like "0xe3af" or decimal string
            cv.Optional(CONF_VERSION):     cv.string,
            cv.Optional(CONF_MDNS):        cv.string,
            # ui_port: 1–65535 only; omit entirely if the device has no web UI
            cv.Optional(CONF_UI_PORT):     cv.int_range(min=1, max=65535),
            cv.Optional(CONF_WEBUI, default=True): cv.boolean,
            cv.Optional(CONF_CAPABILITIES): cv.ensure_list(cv.string),
        }
    ).extend(cv.COMPONENT_SCHEMA),
    validate_has_name,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # 1. Project mapping (from 'esphome: project: name:')
    project = config.get(CONF_PROJECT)
    if project is None:
        if 'esphome' in CORE.config and 'project' in CORE.config['esphome']:
            project = CORE.config['esphome']['project']['name']
    
    if project:
        cg.add(var.set_project(project))

    # 2. Version mapping (from 'esphome: project: version:')
    version = config.get(CONF_VERSION)
    if version is None:
        if 'esphome' in CORE.config and 'project' in CORE.config['esphome']:
            version = CORE.config['esphome']['project']['version']
    
    if version:
        cg.add(var.set_version(version))

    # 3. Name mapping (prefer friendly_name, fall back to name)
    name = config.get(CONF_DEVICE_NAME)
    if name is None:
        if 'esphome' in CORE.config:
            name = CORE.config['esphome'].get('friendly_name') or CORE.config['esphome']['name']
    
    if name:
        cg.add(var.set_name(name))

    # 4. Firmware mapping (ESPHome version string)
    firmware = config.get(CONF_FIRMWARE)
    if firmware is None:
        import esphome.const as esp_const
        firmware = f"ESPHome {esp_const.__version__}"
    
    if firmware:
        cg.add(var.set_firmware(firmware))

    # 5. mDNS mapping
    mdns = config.get(CONF_MDNS)
    if mdns is None:
        if 'mdns' not in CORE.config or not CORE.config['mdns'].get('disabled', False):
            mdns = CORE.config['esphome']['name']  # Just the hostname, no suffix
    
    if mdns:
        cg.add(var.set_mdns(mdns))

    # 6. UI Port mapping (default to 80 if 'web_server' is present)
    ui_port = config.get(CONF_UI_PORT)
    if ui_port is None and config.get(CONF_WEBUI):
        if 'web_server' in CORE.config:
            # Default to 80 since it's the standard for ESPHome web_server
            ui_port = 80
            # If the user has explicitly changed the port in web_server, use that
            if 'port' in CORE.config['web_server']:
                ui_port = CORE.config['web_server']['port']
    
    if ui_port is not None and config.get(CONF_WEBUI):
        cg.add(var.set_ui_port(ui_port))

    # 7. Material symbol (optional)
    if CONF_MATERIAL_SYMBOL in config:
        cg.add(var.set_material_symbol(config[CONF_MATERIAL_SYMBOL]))

    if CONF_CAPABILITIES in config:
        for cap in config[CONF_CAPABILITIES]:
            cg.add(var.add_capability(cap))
