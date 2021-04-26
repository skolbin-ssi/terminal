﻿// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "Launch.h"
#include "Launch.g.cpp"
#include "LaunchPageNavigationState.g.cpp"
#include "EnumEntry.h"

using namespace winrt::Windows::UI::Xaml::Navigation;
using namespace winrt::Windows::Foundation;
using namespace winrt::Microsoft::Terminal::Settings::Model;

namespace winrt::Microsoft::Terminal::Settings::Editor::implementation
{
    Launch::Launch()
    {
        InitializeComponent();

        INITIALIZE_BINDABLE_ENUM_SETTING(LaunchMode, LaunchMode, LaunchMode, L"Globals_LaunchMode", L"Content");
        INITIALIZE_BINDABLE_ENUM_SETTING(WindowingBehavior, WindowingMode, WindowingMode, L"Globals_WindowingBehavior", L"Content");
    }

    void Launch::OnNavigatedTo(const NavigationEventArgs& e)
    {
        _State = e.Parameter().as<Editor::LaunchPageNavigationState>();
    }

    IInspectable Launch::CurrentDefaultProfile()
    {
        const auto defaultProfileGuid{ _State.Settings().GlobalSettings().DefaultProfile() };
        return winrt::box_value(_State.Settings().FindProfile(defaultProfileGuid));
    }

    void Launch::CurrentDefaultProfile(const IInspectable& value)
    {
        const auto profile{ winrt::unbox_value<Model::Profile>(value) };
        _State.Settings().GlobalSettings().DefaultProfile(profile.Guid());
    }

    // TODO GH#9463 - Complete hookup of Terminal UX to choose defapp.
    Windows::Foundation::Collections::IObservableVector<IInspectable> Launch::DefaultTerminals()
    {
        Windows::Foundation::Collections::IObservableVector<IInspectable> vec;
        return vec;
    }

    IInspectable Launch::CurrentDefaultTerminal()
    {
        return nullptr;
    }

    void Launch::CurrentDefaultTerminal(const IInspectable& value)
    {
        value;
    }
}
