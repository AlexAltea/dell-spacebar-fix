# Dell Spacebar Fix

Software solution for double-space issues in Dell XPS 13/15 laptops.


## Usage

1. Download the latest snapshot of [dell-spacebar-fix](https://github.com/AlexAltea/dell-spacebar-fix/archive/master.zip).

2. Extract the contents of the .zip file and go to the *bin* folder.

3. Install the service by executing `install-service.bat` with administrator privileges.

4. Enjoy!

5. Uninstall the service by executing `uninstall-service.bat` with administrator privileges.


## Details

Although Dell has improved the user experience with recent updates, issues related to the infamous spacebar double-press problem are still appearing once in a while. Specifically:

* __Doubled spaces__: Typing "_hello world_" results in "_hello&nbsp;&nbsp;world_".
* __Inserted spaces__: Typing "_hello world_" results in "_hello w orld_".

After monitoring global keyboard events on Windows, I noticed this bug can be uniquely identified in realtime by monitoring keyboard events with `SetWindowsHookEx` and the unwanted events can thus be filtered out. This service is implemented as a background process (running as a regular user application, not a Windows service) which detects those extra spacebar presses and discards them.

The unwanted _double spaces_ are detected based on the following observed pattern, where *Ti* represent an increasing sequence of timestamps.

```
#1  T0  VK_SPACE  WM_KEYDOWN
#2  T0  VK_SPACE  WM_KEYUP
#3  T1  *         WM_KEYUP
#4  T1  VK_SPACE  WM_KEYDOWN
#5  T2  VK_SPACE  WM_KEYUP
```

The unwanted _inserted spaces_ are detected based on the following observed pattern, where *Ti* represent an increasing sequence of timestamps.

```
#1  T0  VK_SPACE  WM_KEYDOWN
#2  T0  VK_SPACE  WM_KEYUP
#3  T1  *         WM_KEYDOWN
#4  T1  VK_SPACE  WM_KEYDOWN
#5  T2  VK_SPACE  WM_KEYUP
```

This service will detect the event patterns (#1,#2) and (#3,#4) and for each occurrence drop the events #2 and #4 respectively. Thus, the final events for both above-mentioned sequences will be:

```
#1  T0  VK_SPACE  WM_KEYDOWN
#3  T1  *         *
#5  T2  VK_SPACE  WM_KEYUP
```
