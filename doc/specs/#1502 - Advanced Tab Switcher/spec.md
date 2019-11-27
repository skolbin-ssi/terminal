---
author: Leon Liang @leonMSFT, Mike Griese @zadjii-msft
created on: 2019-11-27
last updated: 2019-11-27
issue id: 1502
---

# Advanced Tab Switcher

## Abstract

Currently the user is able to cycle through tabs on the tab bar. However, this horizontal cycling can be pretty inconvenient when the tab titles are long or when there are too many tabs on the tab bar. It could also get hard to see all your available tabs if the tab titles are long and your screen is small. In addition, there's a common use case to quickly switch between two tabs, e.g. when one tab is used as reference and the other is the actively worked-on tab. If the tabs are not right next to each other on the tab bar, it could be difficult to quickly swap between the two. Having the tabs displayed in Most Recently Used (MRU) order would help with this problem. It could also make the user experience better when there are a handful of tabs that are frequently used, but are nowhere near each other on the tab bar.

Having a tab switcher UI, like the ones in Visual Studio and Visual Studio Code, could help with the tab experience. Presenting the tabs vertically in their own little UI allows the user to see more of the tabs at once, compared to scanning the tab row horizontally and scrolling left/right to find the tab you want. The tab order in those tab switchers are also in MRU order by default.

To try to alleviate some of these user scenarios, we want to create a tab switcher similar to the ones found in VSCode and VS. This spec will cover the design of the switcher, and how a user would interact with the switcher. It would be primarily keyboard driven, and would give a pop-up display of a vertical list of tabs. The tab switcher would also be able to display the tabs in Most Recently Used (MRU) order.

## Inspiration

This was mainly inspired by the tab switcher that's found in Visual Studio Code and Visual Studio.

VS Code's tab switcher appears directly underneath the tab bar.

![Visual Studio Code Tab Switcher](img/VSCodeTabSwitcher.png)

Visual Studio's tab switcher presents itself as a box in the middle of the editor.

![Visual Studio Tab Switcher](img/VSTabSwitcher.png)

I'm partial towards Visual Studio's implementation, specifically where a box pops up in the center of the window. It allows for expansion in all four directions, which could be useful when adding future features such as Pane Navigation.

In terms of navigating the switcher, both VSCode and Visual Studio behave very similarly. Both open with the press of <kbd>ctrl+tab</kbd> and dismiss on release of <kbd>ctrl</kbd>. They both also allow the user to select the tab with the mouse and with <kbd>enter</kbd>. <kbd>esc</kbd> and a mouse click outside of the switcher both dismiss the window as well.

## Solution Design

In addition to the current in-order tab list, namely `vector<shared_ptr<Tab>> _tabs`, we'll add `vector<weak_ptr<Tab>> _mruTabs` to represent the tabs in Most Recently Used order.
`_mruTabs` will use `weak_ptr` because the tab switcher doesn't actually own the tabs, the `_tabs` vector does.

We'll create a new class `TabSwitcherControl` inside of TerminalPage, which will be initialized with a `vector<>&`.
This is because it'll take in either `_tabs` or `_mruTabs` depending on what order the user would like the tabs to be displayed.
Whenever the user changes the setting that controls the tab order, it'll update to get a reference to the correct vector.

This vector of tabs will be used to create a ListView, so that the UI would diplay a vertical list of tabs, represented by their tab titles.
Each element in the ListView would be associated with a Tab, and the action associated with selecting a tab would be to Focus on that tab.
There would also be another column to the left/right of the tab title column that holds the numbers 1-9. This is to represent what number is bound to each tab to allow for quick switching.
This ListView would be hidden until the user presses a keybinding to show the ListView.

The `TabSwitcherControl` will be able to call any tab's `SetFocused` function to bring the tab into focus.
The TabView's `SelectionChanged` handler listens for events where a new terminal control comes into focus, in which case `TerminalPage::_OnTabSelectionChanged` will be called.
By updating the MRU here, we can be sure that changing tabs from the TabSwitcher, clicking on a tab, or nextTab/prevTab-ing will keep the MRU up-to-date. Adding or closing tabs are handled in `_OpenNewTab` and `_CloseFocusedTab`, which will need to be modified to update both `_tabs` and `_mruTabs`.

## UI/UX Design

First, we'll give a quick overview of how the tab switcher UI would look like, then we'll dive into more detail on how the user would interact with the switcher.

The tab switcher will appear as a box in the center of the terminal. It'll have a maximum and minimum height and width. We want the switcher to stretch, but it shouldn't stretch to take up the entirety of the terminal. If the user has a ton of tabs or has really long tab names, the box should still be fairly contained and shouldn't run wild.  

In the box will be a vertical list of tabs with their titles and their assigned number for quick switching, and only one line will be highlighted to signify that tab is currently selected.
There will only be 9 numbered tabs for quick switching, and the rest of the tabs will simply have an empty space where a number would be.

The list would look (roughly) like this:
```
1 foo (highlighted)
2 boo
3 Windows boofoo
4 /c/Users/booboo
5 Git Moo
6 shoo
7 /c/
8 /d/
9 /e/
  /f/
  /g/
  /h/
```

The highlighted line can move up or down, and if the user moves up while the highlighted line is already at the top of the list, the highlight will wrap around to the bottom of the list.
Similarly, it will wrap to the top if the highlight is at the bottom of the list and the user moves down.

If there's more tabs than the UI can display, the list of tabs will scroll up/down as the user keeps iterating up/down.
However, the number column does not move. The first nine tabs in the list will always be numbered.

To give an example of what happens after scrolling past the end, imagine a user is starting from the state in the mock above.
The user then iterates down past the end of the visible list four times. The below mock shows the result.

```
1 Git Moo
2 shoo
3 /c/
4 /d
5 /e/
6 /f/
7 /g/
8 /h/
9 /i/
  /j/
  /k/
  /l/ (highlighted)
```

`Git Moo` is now associated with number 1, and the `foo`, `boo`, `Windows boofoo`, and `/c/Users/booboo` tabs are no longer visible.

### Using the Switcher

#### Opening the Tab Switcher

The user will press a keybinding named `OpenTabSwitcher` to bring up the UI.
The user will be able to change it to whatever they like. There will also be an optional `anchor` arg that may be provided to this keybinding.

#### Keeping it open

We use the term `anchor` to illustrate the idea that the UI stays visible as long as something is "anchoring" it down.

So, when the `OpenTabSwitcher` keybinding is given the `anchor` arg, the first key of the keybinding will act as the `anchor` key.
Holding that`anchor` key down will keep the switcher visible, and once the `anchor` key is released, the switcher will dismiss.

If `OpenTabSwitcher` is not given the `anchor` arg, the switcher will stay visible even after the release of the key/keychord.

#### Switching through Tabs

The user will be able to navigate through the switcher with the following keybindings:

- Switching Down: <kbd>tab</kbd> or <kbd>downArrow</kbd>
- Switching Up: <kbd>shift+tab</kbd> or <kbd>upArrow</kbd>

As the user is cycling through the tab list, the selected tab will be highlighted but the terminal won't actually switch focus to the selected tab.

#### Closing the Switcher and Bringing a Tab into Focus

There are two _dismissal_ keybinds:

1. <kbd>enter</kbd> : brings the currently selected tab into focus and dismisses the UI.
2. <kbd>esc</kbd> : dismisses the UI without changing tab focus.

The following are ways a user can dismiss the UI, _whether or not_ the `Anchor` arg is provided to `OpenTabSwitcher`.

1. The user can press a number associated with a tab to instantly switch to the tab and dismiss the switcher.
2. The user can click on a tab to instantly switch to the tab and dismiss the switcher.
3. The user can click outside of the UI to dismiss the switcher without bringing the selected tab into focus.
4. The user can press any of the dismissal keybinds.

If the `Anchor` arg is provided, then in addition to the above methods, the UI will dismiss upon the release of the `Anchor` key.

Pressing the `OpenTabSwitcher` keychord again will _not_ close the Switcher.

### Most Recently Used Order

We'll provide a setting that will allow the list of tabs to be presented in either _in-order_ (how the tabs are ordered on the tab bar), or _Most Recently Used Order_ (MRU).
MRU means that the tab that the terminal most recently visited will be on the top of the list, and the tab that the terminal has not visited for the longest time will be on the bottom.

### Numbered Tabs

Similar to how the user can currently switch to a particular tab with a combination of keys such as <kbd>ctrl+shift+1</kbd>, we want to have the tab switcher provide a number to the first nine tabs (1-9) in the list for quick switching. If there are more than nine tabs in the list, then the rest of the tabs will not have a number assigned.

## Capabilities

### Accessibility

- The tab switcher will be using WinUI, and so it'll be automatically linked to the UIA tree. This allows screen readers
to find it, and so narrator will be able to navigate the switcher easily.
- The UI is also fully keyboard-driven, with the option of using a mouse to interact with the UI.
- When the tab switcher pops up, the focus immediately swaps to it.
- For the sake of contrast with the background, we could make it so that it's similar to how the About Page is presented.
Specifically, a box would pop up, and the rest of the terminal would turn white with some transparency so that the focus is very clearly on the tab switcher.

### Security

This shouldn't introduce any security issues.

### Reliability

How we're updating the MRU is something to watch out for since it triggers on a lot of tab interactions. However, I don't foresee
the update taking long at all, and I can't imagine that users can create and delete tabs fast enough to matter.

### Compatibility

- The existing way of navigating horizontally through the tabs on the tab bar should not break.
- These should also be separate keybindings from the keybindings associated with using the tab switcher.
- Reordering tabs on the tab bar shouldn't change the MRU order. The MRU is essentially supposed to be a different _view_ of the already existing tab list.

### Performance, Power, and Efficiency

## Potential Issues

We'll need to be careful about how the UI is presented depending on different sizes of the terminal. We also should test how the UI looks as it's open and resizing is happening.

Visual Studio's tab switcher is a fixed size, and is always in the middle. Even when the VS window is smaller than the tab switcher size, the tab switcher will show up larger than the VS window itself.

![Small Visual Studio Without Tab Switcher](img/VSMinimumSize.png)
![Small Visual Studio With Tab Switcher](img/VSMinimumSizeWithTabSwitcher.png)

Visual Studio Code only allows the user to shrink the window until it hits a minimum width and height. This minimum width and height gives its tab switcher enough space to show a meaningful amount of information.

![Small Visual Studio Code with Tab Switcher](img/VSCodeMinimumTabSwitcherSize.png)

Terminal can't really replicate Visual Studio's version of the tab switcher in this situation. The TabSwitcher needs to be contained within the Terminal.
So, if the TabSwitcher is always centered and has a percentage padding from the borders of the Terminal, it'll shrink as Terminal shrinks.
One thing I'm not too sure about is when the Terminal window is so small that not even a single tab title will be visible. Should it just not display the UI?
Should it try to display the UI and whatever is visible, is visible?

## Future considerations

### Pane Navigation

@zadiji-msft in [#1502] brought up the idea of pane navigation, inspired by tmux.

![Tmux Tab and Pane Switching](img/tmuxPaneSwitching.png)

Tmux allows the user to navigate directly to a pane and even give a preview of the pane. This would be extremely useful since it would
allow the user to see a tree of their open tabs and panes. Currently there's no way to see what panes are open in each tab,
so if you're looking for a particular pane, you'd need to cycle through your tabs to find it. If something like pane profile names (not sure what information to present in the switcher for panes)
were presented in the TabSwitcher, the user could see all the panes in one box.

To support pane navigation, the tab switcher can simply have another column to the right of the tab list to show a list of panes inside the selected tab.
As the user iterates through the tab list, they can simply hit right to dig deeper into the tab's panes, and hit left to come
back to the tab list.

Question: Does MRU make sense for panes?

Pane navigation is a clear next step to build on top of the tab switcher, but this spec will specifically deal with just
tab navigation in order to keep the scope tight. The tab swticher implementation just needs to allow for pane navigation to be added in later.

### Tab Search by Name/Title

This is something that would be particularly nice to have, especially when there's a large list of tabs.
You could be cycling through your thousands of tabs, trying to find the tab that's actually 456th in the list.

A search box would be attached to the UI and the list of tabs that are presented would be filtered based on what the string is in the searchbox.
We already have search being worked on [#605], so we could probably leverage some of that work for tab search.

### Tab Preview on Hover

With this feature, having a tab highlighted in the switcher would make the Terminal display that tab as if it switched to it.
I believe currently there is no way to set focus to a tab in a "preview" mode. This is important because MRU updates whenever a tab
is focused, but we don't want the MRU to update on a preview. Given that this feature is a "nice thing to have", I'll leave it for
after the tab switcher has landed.

## Resources

Feature Request: An advanced tab switcher [#1502]  
Ctrl+Tab toggle between last two windows like Alt+Tab [#973]  
The Command Palette Thread [#2046]
Feature Request: Search [#605]

<!-- Footnotes -->
[#605]: https://github.com/microsoft/terminal/issues/605
[#973]: https://github.com/microsoft/terminal/issues/973
[#1502]: https://github.com/microsoft/terminal/issues/1502
[#2046]: https://github.com/microsoft/terminal/issues/2046