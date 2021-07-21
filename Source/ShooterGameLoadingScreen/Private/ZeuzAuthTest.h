// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class ZeuzAuthTest
{
public:
	ZeuzAuthTest();
	~ZeuzAuthTest();
};

DEFINE_LOG_CATEGORY_STATIC(LogTest, Log, All);

void TestAuth() {
    FZeuzContext& ctx = FZeuzContext::Def;
    ctx.ProjID = "ovDwSBfgjlXNtBOmQqkSNlFAenS";
    ctx.EnvID = "uOJvpWmhzkVByequrudsOxcExaH";

    FZeuzSimpleAuthLoginIn login;
    login.Login = "demo@demo.com";
    login.Password = "demo";

    UZeuzApiSimpleAuth::FDelegateAuthLogin onresult;
    onresult.BindLambda(
        [](FZeuzContext Context, FString Error) {
            if (!Error.IsEmpty())
            {
                UE_LOG(LogTest, Warning, TEXT("Error: %s"), *Error);
            }
            else {
                UE_LOG(LogTest, Warning, TEXT("Success: %s"), *Context.DeveloperID);
            }
        }
    );

    UZeuzApiSimpleAuth::AuthLogin(login, onresult, ctx);
}