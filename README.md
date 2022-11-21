# EngineeringTime Watchface

## Dev

https://rebble.io/hackathon-001/vm/
https://github.com/Willow-Systems/pebble-tool-vscode-extension
https://willow.systems/pebble/

```
pebble build
export DISPLAY=:0.0
pebble install --emulator aplite
pebble kill
pebble wipe
```

## Pebble OG (aplite) Specs

https://developer.rebble.io/developer.pebble.com/guides/tools-and-resources/hardware-information/index.html

### Display

Screen Size: 1.26"
Display Type: E-Paper
Screen Resolution: 144 x 168 px (172 dpi) (175 according to developer.rebble.io)
Supported colors: 2

## Other pebble

```
"targetPlatforms": [
      "aplite",
      "basalt",
      "chalk",
      "diorite"
    ],
```

## Resources and Inspiration

Hershey font:
http://hershey-noailles.luuse.io/www/#
http://paulbourke.net/dataformats/hershey/

Guides:
https://developer.rebble.io/developer.pebble.com/tutorials/watchface-tutorial/part2/index.html
https://pebble.gitbooks.io/learning-c-with-pebble/content/chapter19.html

Examples:
https://github.com/pebble-examples/simple-analog/
https://github.com/yanatan16/pebble-discs
https://github.com/C-D-Lewis/pebble
https://github.com/C-D-Lewis/thin
https://github.com/brunobasto/pebble-react
https://github.com/markusressel/Watchface-No.-2
https://github.com/erikbrinkman/modernesque
https://github.com/snymainn/minimalist-pebble
