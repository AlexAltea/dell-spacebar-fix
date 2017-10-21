# Dell Spacebar Fix

Software solution for double-space issues in Dell XPS 13/15 laptops.


## Usage

1. Download the latest snapshot of [dell-spacebar-fix](https://github.com/AlexAltea/dell-spacebar-fix/archive/master.zip).

2. Extract the contents of the .zip file and go to the `bin` folder.

3. Install the service by executing `install-service.bat`.

4. Enjoy!

5. Uninstall the service by executing `uninstall-service.bat`.


## Details

Although Dell has seemingly improved this issue, there's still the following occurring pattern where *Ti* represent an increasing sequence of timestamps.

```
#1  T0  VK_SPACE  WM_KEYDOWN
#2  T0  VK_SPACE  WM_KEYUP
#3  T1  *         WM_KEYDOWN
#4  T1  VK_SPACE  WM_KEYDOWN
#5  T2  VK_SPACE  WM_KEYUP
```

This service will detect the event patterns (#1,#2) and (#3,#4), and for each occurrence dropping the events #2 and #4, thus the final events for the above-mentioned sequence will be:

```
#1  T0  VK_SPACE  WM_KEYDOWN
#3  T1  *         WM_KEYDOWN
#5  T2  VK_SPACE  WM_KEYUP
```
