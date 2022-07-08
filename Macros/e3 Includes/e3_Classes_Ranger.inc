|----------------------------------------------------------------------------|
| Ranger Functions
|----------------------------------------------------------------------------|
Sub RNG_Setup
  /call iniToVarV "${Character_Ini},${Me.Class},TargetAE Ranged (On/Off)" swapTgtAeRanged bool outer
  /if (${swapTgtAeRanged} && !${Ini[${Character_Ini},Swap Items,Ranged].Length}) {
    /bc TargetAE Ranged (On/Off) = On,  but I have no "Ranged=<item>" in [Swap Item]
    /beep
  }
/return
|----------------------------------------------------------------------------|
SUB RNG_MacroSettings
/if (${Debug}) /echo |- RNG_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},RNG Functions,RNG Function#1" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},RNG Functions,RNG Function#2" "check_Heals" 1
  /call WriteToIni "${advSettings_Ini},RNG Functions,RNG Function#3" "check_Cures" 1
  /call WriteToIni "${advSettings_Ini},RNG Functions,RNG Function#4" "check_AE" 1
	/call WriteToIni "${advSettings_Ini},RNG Functions,RNG Function#5" "check_DoTs" 1
	/call WriteToIni "${advSettings_Ini},RNG Functions,RNG Function#6" "check_Nukes" 1
	/call WriteToIni "${advSettings_Ini},RNG Functions,RNG Function#7" "check_Buffs" 1
  /call WriteToIni "${advSettings_Ini},RNG Functions,RNG Function#8" "check_Food" 1
/if (${Debug}) /echo <== RNG_MacroSettings -|
/return
|----------------------------------------------------------------------------|
SUB RNG_CharacterSettings
  /call WriteToIni "${Character_Ini},${Me.Class},TargetAE Ranged (On/Off)" Off
/return
|----------------------------------------------------------------------------|
Sub RNG_Aliases
/return
|----------------------------------------------------------------------------|
#event AE_AOH "#*# tells you, 'AEAOH'"
sub Event_AE_AOH(string line)
  /if (${Me.Class.ShortName.Equal[RNG]}) {
   /if (${Me.AltAbilityReady[Mass Group Buff]} && ${Me.AltAbilityReady[Auspice of the Hunter]}) {
     /bc MGB Auspice of the Hunter inc...
     /call castSimpleSpell "Mass Group Buff" 0
     /call castSimpleSpell "Auspice of the Hunter" 0
     /rs MGB Auspice of the Hunter inc...
   } else /if (!${Me.AltAbilityReady[Mass Group Buff]}) {
      /bc Mass Group Buff is not available...
   } else /bc Auspice of the Hunter is not available...
  }
/return

SUB check_RangerAggro

  /declare maxAggro int local 90
  |update our max agro on xtargets
  /declare i int local 1
  /declare tempMaxAggro int local 0
  /declare mobId int local 0
  |first we do a quick max agro check
  /for i 1 to 13
    |check off assist exemptions
    /varset mobId ${Me.XTarget[${i}].ID}
    |Do normal checks
    /if (${Bool[${Me.XTarget[${i}]}]} && ${Me.XTarget[${i}].TargetType.Equal[Auto Hater]} && ${Me.XTarget[${i}].Type.Equal[NPC]} && !${Bool[${Me.XTarget[${i}].Name.Find[Corpse]}]}) {
      /if (${Me.XTarget[${i}].Aggressive}) {
        /if (${tempMaxAggro} <  ${Me.XTarget[${i}].PctAggro}) {
          /varset tempMaxAggro ${Me.XTarget[${i}].PctAggro}
        }
      }
    }
  /next i
  
  /if (${tempMaxAggro} >${maxAggro} && ${Assisting}) {
    /bc [+g+] <check_RangerAggro> agro is high at ${maxAggro}%, canceling assist. Recall assist to reengage
    /beep
    /call AssistOff
  }
  /if (!${Asissting} &&  ${Bool[Target.ID]} && ${Target.Aggressive}) {
    /call check_ReadySimple "Jolt"
    /if (${c_Ready}) {
        /call castSimpleSpell "Jolt" 0
       /stand
    }
  }

/return

|----------------------------------------------------------------------------|
Sub RNG_Background_Events
  /doevents AE_AOH
/return