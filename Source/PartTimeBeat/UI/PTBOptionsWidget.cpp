#include "PTBOptionsWidget.h"

UPTBOptionsWidget::UPTBOptionsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RestoreMode = EPTBMenuRestoreMode::UIOnly;
}