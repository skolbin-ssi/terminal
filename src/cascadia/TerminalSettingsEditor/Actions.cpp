﻿// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "Actions.h"
#include "Actions.g.cpp"
#include "ActionsPageNavigationState.g.cpp"
#include "EnumEntry.h"

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::System;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Xaml::Navigation;
using namespace winrt::Microsoft::Terminal::Settings::Model;

namespace winrt::Microsoft::Terminal::Settings::Editor::implementation
{
    Actions::Actions()
    {
        InitializeComponent();

        _filteredActions = winrt::single_threaded_observable_vector<winrt::Microsoft::Terminal::Settings::Model::Command>();
    }

    void Actions::OnNavigatedTo(const NavigationEventArgs& e)
    {
        _State = e.Parameter().as<Editor::ActionsPageNavigationState>();

        for (const auto& [k, command] : _State.Settings().GlobalSettings().Commands())
        {
            // Filter out nested commands, and commands that aren't bound to a
            // key. This page is currently just for displaying the actions that
            // _are_ bound to keys.
            if (command.HasNestedCommands() || command.KeyChordText().empty())
            {
                continue;
            }
            _filteredActions.Append(command);
        }
    }

    Collections::IObservableVector<Command> Actions::FilteredActions()
    {
        return _filteredActions;
    }

    void Actions::_OpenSettingsClick(const IInspectable& /*sender*/,
                                     const Windows::UI::Xaml::RoutedEventArgs& /*eventArgs*/)
    {
        const CoreWindow window = CoreWindow::GetForCurrentThread();
        const auto rAltState = window.GetKeyState(VirtualKey::RightMenu);
        const auto lAltState = window.GetKeyState(VirtualKey::LeftMenu);
        const bool altPressed = WI_IsFlagSet(lAltState, CoreVirtualKeyStates::Down) ||
                                WI_IsFlagSet(rAltState, CoreVirtualKeyStates::Down);

        const auto target = altPressed ? SettingsTarget::DefaultsFile : SettingsTarget::SettingsFile;

        _State.RequestOpenJson(target);
    }

}
