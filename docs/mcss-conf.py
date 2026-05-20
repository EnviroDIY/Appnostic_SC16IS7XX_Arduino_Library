DOXYFILE = "doxyfile"
HTML_OUTPUT = "../SC16IS7XX_Doxygen/m.css"
WARN_LOGFILE = "output_mcss.log"
PROJECT_REPOSITORY = "https://github.com/EnviroDIY/SC16IS7XX"
THEME_COLOR = "#cb4b16"
FAVICON = "enviroDIY_Favicon.png"
LINKS_NAVBAR1 = [
    (
        "About",
        "index",
        [
            ('<a href="page_device_addresses.html">Possible Device Addresses</a>',),
            ('<a href="page_supported_interrupts.html">Supported Interrupts</a>',),
            ('<a href="change_log.html">ChangeLog</a>',),
        ],
    ),
    (
        "Classes",
        "annotated",
        [],
    ),
    (
        "Source Files",
        "files",
        [],
    ),
    (
        "Examples",
        "page_the_examples",
        [
            (
                '<a href="example_basic_i2c_receive.html">Simple I2C UART receive test</a>',
            ),
            ('<a href="example_basic_i2c_send.html">Simple I2C UART send test</a>',),
            ('<a href="example_gpio_blink.html">Simple GPIO blink test</a>',),
            ('<a href="example_gpio_interrupt.html">Simple GPIO interrupt test</a>',),
            ('<a href="example_gpio_poll.html">Simple GPIO polling test</a>',),
            ('<a href="example_i2c_loopback.html">Simple I2C UART loopback test</a>',),
        ],
    ),
]
LINKS_NAVBAR2 = []
VERSION_LABELS = True
CLASS_INDEX_EXPAND_LEVELS = 2

STYLESHEETS = [
    "css/m-EnviroDIY+documentation.compiled.css",
]
EXTRA_FILES = [
    "gp-desktop-logo.png",
    "gp-mobile-logo.png",
    "gp-scrolling-logo.png",
    "clipboard.js",
]
DESKTOP_LOGO = "gp-desktop-logo.png"
MOBILE_LOGO = "gp-mobile-logo.png"
SCROLLING_LOGO = "gp-scrolling-logo.png"
