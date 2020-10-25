// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

#include "CP0CharacterMovement.h"
#include "BindInputAction.h"

// TODO: 이 코드는 아예 서버에서 실행되는 게 나을 것 같다
// BindInputAction을 Character로 다시 옮기고, 이 코드가 서버에서 호출되도록 만들자
void UCP0CharacterMovement::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    BindInputAction(PlayerInputComponent, TEXT("Sprint"), this, FSprintAction{});
}
