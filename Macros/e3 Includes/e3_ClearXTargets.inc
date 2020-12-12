|--------------------------------------------------------------------------------|
| Clear XTargets
|
| Author: Sirhopsalot
| Contributor: Forsy
|
| Starts assist on each of the targets in your XTarget list until they're all Cleared using the criteria below
| This will cause the toon to stick to the current target, face the mob if looking the wrong way, and re-stick if the mob is too far away
| Calling Clear XTargets again while still clearing toggles it back off
|
| WARNING: This macro is not for AFK grinding. AFK grinding using this or any other macro is against the rules for Lazarus. Please don't do it.
|
| Selects the next target from the XTarget list using the following criteria:
| 1) Lowest aggro on the list
| 2) Highest health on the list
| 2a) Prefers non-Warrior targets with the same aggro over Warrior targets (kill healers/casters/rogues/etc first)

| Usage to clear everything:
|    /bc Clear XTargets
| Usage to clear everything and seek aggro:
|    /bc Clear XTargets KeepAggro
| Usage to select just the next target using the criteria (useful for switching to adds on boss fights)
|    /bc Next Target
|
| Installation:
| 1) Put e3_ClearXTargets.inc in the e3 Includes folder
| 2) In e3_Setup.inc, add: #include e3 Includes\e3_ClearXTargets.inc
| 3) In Advanced Settings.ini, add: Include#21=clearXTargets under [Included Setups]
|--------------------------------------------------------------------------------|

|--------------------------------------------------------------------------------|
| Declare variables used in the macros
|--------------------------------------------------------------------------------|
SUB clearXTargets_Setup
	/declare ClearXTargetsOn bool outer FALSE
	/declare ClearXTargetsDebug bool outer FALSE
  /declare NumAggroTargets int outer 0
  /declare KeepAggroOn bool outer FALSE

  /if (!${Defined[FollowAfterClearXTargets]}) {
    /if (${IniExt[${Character_Ini},Misc,FollowAfterClearXTargets].Count} > 0) {
      /call iniToVarV "${Character_Ini},Misc,FollowAfterClearXTargets" FollowAfterClearXTargets bool outer
    } else {
      /declare FollowAfterClearXTargets bool outer TRUE
    }
  }

  /if (${ClearXTargetsDebug}) /echo FollowAfterClearXTargets = ${FollowAfterClearXTargets}

  /if (!${Defined[ClearXTargetStickDistance]}) {
    /if (${IniExt[${Character_Ini},Misc,ClearXTargetStickDistance].Count} > 0) {
      /call iniToVarV "${Character_Ini},Misc,ClearXTargetStickDistance" ClearXTargetStickDistance int outer
    } else {
      /declare ClearXTargetStickDistance int outer 10
    }
  }

  /if (${ClearXTargetsDebug}) /echo ClearXTargetStickDistance = ${ClearXTargetStickDistance}
/RETURN

|--------------------------------------------------------------------------------|
| Call background events and other subs required for checking the macros
|--------------------------------------------------------------------------------|
SUB clearXTargets_Background_Events
	/doevents clearXTargets
	/doevents nextTarget
  /doevents rotateTarget
	/call check_clearXTargets
/RETURN

SUB clearXTargets_MacroSettings
/RETURN

SUB clearXTargets_CharacterSettings
/RETURN

SUB clearXTargets_Aliases
  /squelch /alias /ClearXTargets /bc Clear XTargets
  /squelch /alias /NextTarget /bc Next Target
/RETURN


| Toggles clearing on/off
#event clearXTargets "<#1#> Clear XTarget#2#"
SUB event_clearXTargets(line, ChatSender, args)
  /if (!${ChatSender.Equal[${Me.Name}]}) /return

  /if (${Bool[${args.Find[KeepAggro]}]}) {
    /if (${ClearXTargetsDebug}) /echo KeepAggro On
    /varset KeepAggroOn TRUE
  } else {
    /if (${ClearXTargetsDebug}) /echo KeepAggro Off
    /varset KeepAggroOn FALSE
  }

  | toggle ClearXTargetsOn on/off 
  /if (!${ClearXTargetsOn}) {
    /varset ClearXTargetsOn TRUE
    | Disable follow to stop melee/ranged toons bouncing
    /stop
  	/echo ClearXTargets: Enabled ${ClearXTargetsOn}
  } else {
    /varset ClearXTargetsOn FALSE
	  /echo ClearXTargets: Disabled
  }
/RETURN

| Selects the next target and attacks
#event nextTarget "<#1#> Next Target"
SUB event_nextTarget(line, ChatSender)
  /if (!${ChatSender.Equal[${Me.Name}]}) /return
  /declare i int local
  /declare worstAggro int local 101
  /declare mobId int local
  /declare mobClass string local Warrior
  /declare mobHealth int local
  /for i 1 to 13
    /if (${Me.XTarget[${i}].TargetType.Equal[Auto Hater]} && (${Me.XTarget[${i}].PctAggro} < ${worstAggro} || (${Me.XTarget[${i}].PctAggro} == ${worstAggro} && ${mobClass.Equal[Warrior]} && ${Me.XTarget[${i}].PctHPs} >= ${mobHealth})) && !${Bool[${Me.XTarget[${i}].Name.Find[Corpse]}]} && !${Bool[${Me.XTarget[${i}].Name.Find[wooden box]}]} && !${Bool[${Me.XTarget[${i}].Name.Find[Chest]}]} && !${Bool[${Me.XTarget[${i}].Name.Find[a barrel]}]} && !${Bool[${Me.XTarget[${i}].Name.Find[a box]}]} && !${Bool[${Me.XTarget[${i}].Name.Find[crate]}]} && !${Bool[${Me.XTarget[${i}].Name.Find[hollow_tree]}]} && ${Me.XTarget[${i}].LineOfSight} && ${Me.XTarget[${i}].Distance} < 75) {
      /varset worstAggro ${Me.XTarget[${i}].PctAggro}
      /varset mobId ${Me.XTarget[${i}].ID}
      /varset mobClass ${Me.XTarget[${i}].Class}
      /varset mobHealth ${Me.XTarget[${i}].PctHPs}
    }
  /next i

  /if (${Bool[${mobId}]} && ${Bool[${Spawn[${mobId}]}]}) {
    /if (${ClearXTargetsDebug}) /echo Assisting on Next Target ${Spawn[${mobId}].CleanName} TargetID=${mobId}
    /call TrueTarget ${mobId}
    /stick id ${mobId} ${ClearXTargetStickDistance} uw
    /delay 1
    /attack on
    /delay 1
    /bcga //pet attack
    /delay 1
    /assistme
    /varset AllowControl TRUE
    /varset AssistStickDistance ${ClearXTargetStickDistance}
  } else {
    /if (${ClearXTargetsDebug}) /echo No mobs on XTarget to assist on
  }

  /doevent flush event_nextTarget
/RETURN

| Face the target if you can't see it
#event faceTargetClearXTargets "You cannot see your target."
SUB event_faceTargetClearXTargets
	/if (${Bool[${AssistTarget}]}) {
		/doevent flush event_faceTargetClearXTargets
		/if (${ClearXTargetsDebug}) /echo ClearXTargets: Changing face direction to target ${Spawn[id ${AssistTarget}].CleanName}
		/face fast id ${AssistTarget}    
	}
/RETURN

| Re-stick if too far away msg   	            
#event getCloserClearXTargets "Your target is too far away, get closer!"
SUB event_getCloserClearXTargets
	/if (${Bool[${AssistTarget}]}) {
		/doevent flush event_getCloserClearXTargets
		/if (${ClearXTargetsDebug}) /echo ClearXTargets: Re-sticking to target ${Spawn[id ${AssistTarget}].CleanName}
		/stick id ${AssistTarget} 5 uw
	}
/RETURN

| Triggers the Next Target or disable Clear XTarget as appropriate
SUB check_clearXTargets
  | Count the current number of aggro targets
  /varset NumAggroTargets 0
  /declare numLowAggroTargets int local 0
  /declare i int local 1
  /for i 1 to 13
    /if (${Me.XTarget[${i}].TargetType.Equal[Auto Hater]} && ${Me.XTarget[${i}].Aggressive}) {
      /varset NumAggroTargets ${Math.Calc[${NumAggroTargets} + 1]}
      /if (${Me.XTarget[${i}].PctAggro} < 100) {
        /varset numLowAggroTargets ${Math.Calc[${numLowAggroTargets} + 1]}
      }
    }
  /next i

  /if (!${ClearXTargetsOn}) /return

  | Check if can't see target and re-stick
  /doevent faceTargetClearXTargets
  /doevent getCloserClearXTargets

  /if (!${Assisting} || ${Target.ID} == ${Me.ID}) {
    | If there are any aggro targets call Next Target
    | If there are none, disable Clear XTargets
    /if (${NumAggroTargets} > 0) {
      /bc Next Target
      /return
    }

  	| No target found, disable clearing
    /varset ClearXTargetsOn FALSE    
    /echo ClearXTargets: XTargets Cleared, disabled Clear XTarget
    /if (${FollowAfterClearXTargets}) {
      | If set to follow after clearing
      /followme
    }
    /attack off
  }

  /if (${KeepAggroOn} && ${numLowAggroTargets} >= 1 && ${Target.PctAggro} >= 100) {
    /bc Next Target
  }
/RETURN

| Selects the next target and attacks
#event rotateTarget "<#1#> Rotate Target"
SUB event_rotateTarget(line, ChatSender)
  /if (!${ChatSender.Equal[${Me.Name}]} || ${NumAggroTargets} == 0) /return
  /declare i int local
  /declare mobId int local
  /declare xtarIndex int local
  /declare newTargetId int local

  /if (${Bool[${AssistTarget}]}) {
    | Find the current target index
    /for i 1 to 13
      /if (${AssistTarget} == ${Me.XTarget[${i}].ID}) {
        /if (${ClearXTargetsDebug}) /echo Current target index = ${i} with name: ${Me.XTarget[${i}].Name}
        /varset xtarIndex ${Math.Calc[${i} + 1]}
        | If we're at the last toon on the list, start at the beginning
        /if (${i} == 13 || ${Me.XTarget[${Math.Calc[${i} + 1]}].ID} == 0) {
          /varset xtarIndex 1
        }
        /if (${ClearXTargetsDebug}) /echo Searching for rotate target starting at xtarIndex = ${xtarIndex}
        /break
      }
    /next i    
  } else {
    | Set the index to 1
    /varset xtarIndex 1
  }

  | find the next valid target in the list
  /for i ${xtarIndex} to 13
    /if (${Me.XTarget[${i}].TargetType.Equal[Auto Hater]} && !${Bool[${Me.XTarget[${i}].Name.Find[Corpse]}]} && !${Bool[${Me.XTarget[${i}].Name.Find[wooden box]}]} && !${Bool[${Me.XTarget[${i}].Name.Find[Chest]}]} && !${Bool[${Me.XTarget[${i}].Name.Find[a barrel]}]} && !${Bool[${Me.XTarget[${i}].Name.Find[a box]}]} && !${Bool[${Me.XTarget[${i}].Name.Find[crate]}]} && !${Bool[${Me.XTarget[${i}].Name.Find[hollow_tree]}]} && ${Me.XTarget[${i}].LineOfSight} && ${Me.XTarget[${i}].Distance} < 75) {
      /if (${ClearXTargetsDebug}) /echo Switching to target at index = ${i} with name: ${Me.XTarget[${i}].Name}
      /varset mobId ${Me.XTarget[${i}].ID}
      /if (${ClearXTargetsDebug}) /echo Switching to target mob with id = ${mobId}
      /break
    }
  /next i

  /if (${Bool[${mobId}]} && ${Bool[${Spawn[${mobId}]}]}) {
    /if (${ClearXTargetsDebug}) /echo Assisting on Next Rotate Target ${Spawn[${mobId}].CleanName} TargetID=${mobId}
    /call TrueTarget ${mobId}
    /stick id ${mobId} ${ClearXTargetStickDistance} uw
    /delay 1
    /attack on
    /delay 1
    /bcga //pet attack
    /delay 1
    /assistme
    /varset AllowControl TRUE
    /varset AssistStickDistance ${ClearXTargetStickDistance}
  } else {
    /if (${ClearXTargetsDebug}) /echo No mobs on XTarget to assist on
  }

  /doevent flush event_rotateTarget
/RETURN