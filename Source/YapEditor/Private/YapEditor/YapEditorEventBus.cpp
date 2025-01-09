#include "YapEditor/YapEditorEventBus.h"

#include <any>

#define LOCTEXT_NAMESPACE "YapEditor"

void FYapEditorEventBus::operator<<(FName EventName)
{
	Events.Add(EventName, {});
}

#undef LOCTEXT_NAMESPACE