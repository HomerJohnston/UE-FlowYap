// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#define LOCTEXT_NAMESPACE "YapEditor"

enum class EYapEventSubscription : uint8
{
	Unspecified,
	Subscribe,
	Ignore,
};

// ----------------------------------------------
template<typename ClassType>
struct FYapEdEventArg
{
	EYapEventSubscription Setting = EYapEventSubscription::Unspecified;

	typename TMemFunPtrType<false, ClassType, void()>::Type Function;
	
	void Ignore()
	{
		Setting = EYapEventSubscription::Ignore;
	}
	
	void Subscribe(typename TMemFunPtrType<false, ClassType, void()>::Type InFunction)
	{
		Setting = EYapEventSubscription::Subscribe;
		Function = InFunction;
	}
};

// ----------------------------------------------
struct FYapEditorEventBus
{
	TMap<FName, TMulticastDelegate<void()>> Events;

	void operator<<(FName Event);
};

// ----------------------------------------------
struct FYapEditorEventSubscriptionArgs
{
	/*FYapEdEventArg ConditionsArrayNumChange;*/
};

class SomeWidget
{
public:
	
	void OnSomeVoidEvent();
	
	void SomeInitFunction()
	{
		//FYapEditorEventSubscriptionArgs Evts;

		//std::function<void(SomeWidget&)> func = &SomeWidget::OnSomeVoidEvent;
		
		//Evts.ConditionsArrayNumChange.Subscribe(&SomeWidget::OnSomeVoidEvent);

		FYapEdEventArg<SomeWidget> Thing;

		TMemFunPtrType<false, SomeWidget, void()>::Type Function = &SomeWidget::OnSomeVoidEvent;
		
		Thing.Function = Function;

		(*this.*Thing.Function)();

		FYapEditorEventBus Bus;

		//Bus.Events[EYapDialogueEvent::Event1].AddRaw(this, &SomeWidget::OnSomeVoidEvent);
	}

	void SomeSubscribeFunction()
	{
		FYapEditorEventBus TestBus;
	}

	void SomeDoStuffFunction()
	{
		{
			//FYapScopedTransaction T(TEXT("Test"), LOCTEXT("TODO", "Info"), nullptr, EYapDialogueEvent::Event1, true);

			// Blah blah

			// Here, the scoped transaction will automatically broadcast the event
		}
	}
	
	void SomeResponseFunction();
};

/*
* using FGetter = TMemFunPtrType<true, UText3DComponent, UMaterialInterface*()>::Type;
	static TMap<FName, FGetter> GroupToMaterial =
	{
		{ TEXT("FrontMaterial"), &UText3DComponent::GetFrontMaterial },
		{ TEXT("BackMaterial"), &UText3DComponent::GetBackMaterial },
		{ TEXT("ExtrudeMaterial"), &UText3DComponent::GetExtrudeMaterial },
		{ TEXT("BevelMaterial"), &UText3DComponent::GetBevelMaterial }
	};
 */

#undef YAP_EVENT

#undef LOCTEXT_NAMESPACE