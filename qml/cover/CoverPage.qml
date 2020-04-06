import QtQuick 2.2
import Sailfish.Silica 1.0

CoverBackground {
    CoverPlaceholder {
        icon.source: Theme.colorScheme === Theme.LightOnDark
                     ? "icon-cover-light.svg"
                     : "icon-cover-dark.svg"
        icon.sourceSize.width: Theme.iconSizeLauncher
        icon.sourceSize.height: Theme.iconSizeLauncher
        text: "Sapphire"
    }
}
