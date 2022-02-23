|--------------------------------------------------------------------|
|- Contains assisting related functions for e3.mac.					        -|
|- check_IsLookingAt by genbot.mac                                  -|
|- Use, 'Assist on ${Target.ID}' in /tell, /group, or /bc.					-|
|--------------------------------------------------------------------|
#event Assist "<#1#> Assist on #2#"
SUB event_Assist(line, ChatSender, int MobID)

/if (${Debug} || ${Debug_Assists}) /echo |- event_Assist ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${MobID}]}) /return
  /varset MobID ${c_eventArgData}
  | Check that you're not assisting on a corpse, pet or pc
  /if (!${Bool[${Spawn[id ${MobID}].Type.NotEqual[Corpse]}]} || !${Select[${Spawn[id ${MobID}].Type},${AcceptableTargetTypes}]}) {
    /if (${TogglePvP}) /goto :assistAnyway
    /if (${Debug} || ${Debug_Assists}) /echo [${Spawn[id ${MobID}].CleanName}] is not an acceptable TargetType
  } else {
    :assistAnyway
    /if (${Me.Feigning}) {
      |/if (${Select[${Me.Class.ShortName},NEC,SHD,MNK]}) {
      /if (${Select[${Me.Class.ShortName},NEC,SHD]}) {
        /return
      } else {
        /stand
      }
    }
    | Check target distance.
    /if (${Spawn[${MobID}].Distance3D} > ${MaxAssistDistance}) {
      /docommand ${ChatToggle} ${Spawn[${MobID}].CleanName} is too far away.
    } else {
      | Wait to catch up to group, before engage.
      |/echo ${Following}   ${Spawn[${FollowTarget}].Distance}   ${LeashLength}   ${Me.Moving}
      /if (${Following} && ${Spawn[${FollowTarget}].Distance3D} > ${LeashLength} && ${Me.Moving}) {
      |/echo distloop
        /declare Assist_Moving_Timer timer local 100
        :movetoAssist_Loop
        /call Background_Events
        /if (${Spawn[=${FollowTarget}].Distance} > ${LeashLength} && ${Assist_Moving_Timer}) /goto :movetoAssist_Loop
      }
      | Disengage follow.
      /if (${Stick.Active}) /squelch /stick off
      /if (${NetAdvPath.Following}) /squelch /netfollow off
      | Set Assist Variables.
      /varset Assisting TRUE
      /varset AssistTarget ${MobID}
      /varset AllowControl FALSE
      /varset isEnraged FALSE
      | If I am the assist caller, or I am feigning, don't allow macro control.
      /if (${ChatSender.NotEqual[${Me.CleanName}]}) {
        /varset AllowControl TRUE
        |/docommand ${ChatToggle} Attacking - ${AssistTarget} - ${Spawn[${AssistTarget}].CleanName}
        /if (${Target.ID} != ${AssistTarget} && (!${Bool[${Me.Casting}]} || ${Me.Class.ShortName.Equal[BRD]}) && (${AssistType.Equal[melee]} || ${AssistType.Equal[ranged]})) /call TrueTarget ${AssistTarget}
        /if (${Me.Pet.ID}) {
          /if (${Debug} || ${Debug_Assists}) /echo Pet Check: Sending pet in
          /pet attack ${AssistTarget}
        }
        /if (${AssistType.Equal[melee]}) {
          /if (${Debug} || ${Debug_Assists}) /echo melee on
          /if (${Int[${AssistDistanceSetting}]}) {
            /varset AssistStickDistance ${AssistDistanceSetting}
          } else {
            /if (${AssistDistanceSetting.Equal[MaxMelee]} || ${AssistDistanceSetting.Equal[auto]}) /varset AssistStickDistance ${Math.Calc[${Spawn[${Target.ID}].MaxRangeTo}*.75].Int}
          }
          | Check MaxRangeTo, make sure it is not out of bounds.
          /if (${AssistStickDistance} > 25) /varset AssistStickDistance 25
          /call StickToAssistTarget
          /if (${Defined[strikeDisc]}) {
            /call check_rogStrike
          } else {
            /attack on
          }
        } else /if (${AssistType.Equal[ranged]}) {
          /if (${Debug} || ${Debug_Assists}) /echo autofire on
          /squelch /face fast id ${AssistTarget}
          /if (!${Me.AutoFire}) /autofire on
          /if (${RangedStickDistance.Equal[Clamp]}) {
            /if (${Target.Distance} >=30 && ${Target.Distance} <=200) {
              /squelch /stick hold moveback ${Target.Distance}
            } else {
              /if (${Target.Distance} > 200) /squelch /stick hold moveback 195
              /if (${Target.Distance} < 30) /squelch /stick hold moveback 35
            }
          } else {
            /squelch /stick hold moveback ${RangedStickDistance}
          }
        } else /if (${AssistType.Equal[off]}) {
          | Face the target for less dmg and stun resist for Ogres
          /squelch /face fast id ${AssistTarget}
        }
      /if (${Debug} || ${Debug_Assists}) /echo Assist=${Assisting} || AssistTarget=${AssistTarget} || AllowControl=${AllowControl}
      }
    }
  }
/if (${Debug} || ${Debug_Assists}) /echo <== event_Assist -|
/RETURN

|--------------------------------------------------------------------|
|- Backs off an AssistTarget, and labels them as a 'PauseTarget'.	  -|
|- You must manually call engage to re-engage a PauseTarget.		    -|
|- Use, '/backoff', or, 'Back off' in /tell, /group, or /bc.		    -|
|--------------------------------------------------------------------|
#event BackOff "<#1#> Back Off#*#"
SUB Event_BackOff(line, ChatSender)
/if (${Debug} || ${Debug_Assists}) /echo |- Event_BackOff ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZR,""]}) /return
|#event BackOff "<#1#> Back Off#2#"
|SUB Event_BackOff(line, ChatSender, args)
|/if (${Debug} || ${Debug_Assists}) /echo |- Event_BackOff ==>

|  /if (!${checkEventArgs[${ChatSender},${line},UZR,${args}]}) /return
  /echo Backing off.
  /if (${c_eventArgChatSender.Equal[${Me}]}) /docommand ${ChatToggle} Backing off.
  /varset PauseTarget ${Target.ID}
  /call AssistOff
/if (${Debug} || ${Debug_Assists}) /echo <== Event_BackOff -|
/RETURN

|----------------------------------------------------------------|
|- Turns off Assist_Functions, and resets assisting variables.	-|
|----------------------------------------------------------------|
SUB AssistOff
/if (${Debug} || ${Debug_Assists}) /echo |- AssistOff -| ==>
  /if (${Me.Casting.ID} && !${Spell[${Me.Casting}].SpellType.Find[beneficial]} && ${Me.Class.ShortName.NotEqual[BRD]}) /call interrupt
  /if (${Me.Combat})    /attack off
  /if (${Me.AutoFire})  /autofire off
  /if (${Me.Pet.ID}) /squelch /pet back off
  /delay 5 !${Me.Combat} && !${Me.AutoFire}

	/varset Assisting FALSE
	/varset AssistTarget 0
	/varset AllowControl FALSE
  /varset isEnraged FALSE

  /if (${Stick.Status.Equal[ON]})   /squelch /stick off
  |/if (${Defined[Debuff_Targets]})  /deletevar Debuff_Targets
  |/if (${Defined[DoT_Targets]})     /deletevar DoT_Targets
  /if (${use_FULLBurns})           /varset use_FULLBurns FALSE
  /if (${use_LONGBurns})           /varset use_LONGBurns FALSE
  /if (${use_QUICKBurns})          /varset use_QUICKBurns FALSE
	/if (${use_EPICBurns})           /varset use_EPICBurns FALSE

  /if (${Defined[${resistCount}]}) /deletevar ${resistCount}
  /call check_ResistCounters
  /if (${Following}) /call AcquireFollow

/if (${Debug} || ${Debug_Assists}) /echo |- AssistOff -| <== [${Assisting}] || AssistTarget: [${AssistTarget}]
/RETURN

|----------------------------------------|
|- Engages /stick on an AssistTarget.	-|
|----------------------------------------|
SUB StickToAssistTarget
/if (${Debug} || ${Debug_Assists}) /echo |- StickToAssistTarget ==> assistPoint ${AssistStickPoint} dist ${AssistStickDistance}
		/if (${AssistStickPoint.Equal[behind]}) {
			/if (${Debug} || ${Debug_Assists}) /echo Attempting to stick behind [${AssistStickDistance}].
			/squelch /stick snaproll uw
			/delay 20 ${Stick.Behind} && ${Stick.Stopped}
			/squelch /stick hold moveback behind ${AssistStickDistance} uw
    } else /if (${AssistStickPoint.Equal[front]}) {
      /if (${Debug} || ${Debug_Assists}) /echo Attempting to stick front [${AssistStickDistance}].
      /squelch /stick hold front ${AssistStickDistance} uw
      /delay 20 ${Stick.Stopped}
		} else /if (${AssistStickPoint.Equal[behindonce]}) {
			/if (${Debug} || ${Debug_Assists}) /echo Attempting to stick behindonce [${AssistStickDistance}].
			/squelch /stick snaproll uw
      /delay 20 ${Stick.Behind} && ${Stick.Stopped}
			/squelch /stick hold moveback behindonce ${AssistStickDistance} uw
		} else /if (${AssistStickPoint.Equal[pin]}) {
			/if (${Debug} || ${Debug_Assists}) /echo Attempting to stick pin [${AssistStickDistance}].
			/squelch /stick snaproll uw
			/delay 20 ${Stick.Stopped}
			/squelch /stick hold moveback pin ${AssistStickDistance} uw
		} else /if (${AssistStickPoint.Equal[!front]}) {
			/if (${Debug} || ${Debug_Assists}) /echo Attempting to stick !front [${AssistStickDistance}].
			/squelch /stick snaproll uw
      /delay 20 ${Stick.Behind} && ${Stick.Stopped}
			/squelch stick hold moveback !front ${AssistStickDistance} uw
		} else {
			/if (${Debug} || ${Debug_Assists}) /echo [${AssistStickPoint}] is not a valid stick point.  Defaulting to stick behind.
			/squelch /stick snaproll uw
      /delay 20 ${Stick.Behind} && ${Stick.Stopped}
			/squelch /stick hold moveback behind ${AssistStickDistance} uw
		}
/if (${Debug} || ${Debug_Assists}) /echo <== StickToAssistTarget -|	
/RETURN


|----------------------------------------------------|
|- Checks range and combat status while assisting.	-|
|----------------------------------------------------|
SUB check_AssistStatus
/if (${Debug} || ${Debug_Assists}) {
	/echo |- check_AssistStatus ==> AssistTarget = [${AssistTarget}] || AllowControl = [${AllowControl}] || Distance check = [${Spawn[id ${AssistTarget}].Distance} < ${MaxAssistDistance}] || TargetType = [${Spawn[id ${AssistTarget}].Type}]
}
  /if (!${Bool[${Spawn[id ${AssistTarget}].Type.NotEqual[Corpse]}]}) {
    /if (${Debug} || ${Debug_Assists}) /echo Target doesnt exist or is dead, AssistOff
    /call AssistOff
    /return
  }
  |only if macro controlled and assisting, stand up from FD
  /if (${Me.Feigning}) {
    /if (${AllowControl} || !${Select[${Me.Class.ShortName},NEC,SHD,MNK]}) /stand
  }
  | If the character is macro controlled
  /if (${AllowControl} && ((${AssistType.Equal[melee]} || ${AssistType.Equal[ranged]}))) {
    | Check Target: this only forces retarget for melee/autofire bots and lets casters/priests do their thing
    /if (${Target.ID} != ${AssistTarget} && (!${Bool[${Me.Casting}]} || ${Me.Class.ShortName.Equal[BRD]}) && (${AssistType.Equal[melee]} || ${AssistType.Equal[ranged]})) /call TrueTarget ${AssistTarget}
    |----- Melee functions-------------------------------------------------------------------------------------------------------------|
      /if (${AssistType.Equal[Melee]} || ${pauseRanged}) {
        /if (${isEnraged}) {
          /squelch /face fast
          /if ((${Math.Abs[${Me.Heading.Degrees}-${Target.Heading.Degrees}]} > 85 && !${Stick.Behind}) || ${AssistStickPoint.Equal[front]}) {
            /if (${Debug} || ${Debug_Assists}) /echo I am not behind the target. Stopping attack.
            /attack off
            /return
          }
        }
        | If I am target of target and not sticking to front(tank) attack/stick off
        | If I am < 50% life turn off attack (unless tank/stick front)
        /if (${Me.TargetOfTarget.CleanName.Equal[${Me.CleanName}]} && ${AssistStickPoint.NotEqual[front]}) {
          /if (${Me.PctHPs} < 80 && !${Bool[${Select[${Me.TargetOfTarget.Class.ShortName},WAR,PAL,SHD]}]}) /attack off
          /stick off
        } else {
          | If the mob is not targeting me
          /if (!${Me.Combat}) /attack on
          /if (!${Stick.Active} || (${Target.Distance} > ${AssistStickDistance}) || (${AssistStickPoint.Find[behind]} && !${Stick.Behind})) {
            /call StickToAssistTarget
            /doevents getCloser
          }
          /if (${pauseRanged}) /varset pauseRanged FALSE
        }
    |----- Ranged functions-------------------------------------------------------------------------------------------------------------||
    } else /if (${AssistType.Equal[Ranged]} && !${pauseRanged}) {
      /squelch /face fast id ${AssistTarget}
      |dont play chase
      /if (${Me.TargetOfTarget.CleanName.Equal[${Me.CleanName}]}) {
        /squelch /autofire off
        /squelch /attack on
        /varset pauseRanged TRUE
        /squelch /stick 8
      } else {
        /if (${RangedStickDistance.Equal[Clamp]}) {
          /if (${Target.Distance} >=30 && ${Target.Distance} <=200) {
            /squelch /stick hold moveback ${Target.Distance}
          } else {
            /if (${Target.Distance} > 200) /squelch /stick hold moveback 195
            /if (${Target.Distance} < 30) /squelch /stick hold moveback 35
          }
        } else {
          /squelch /stick hold moveback ${RangedStickDistance}
        }
        /if (!${Me.AutoFire}) /autofire on
      }
    }
  }
  /if ((!${Me.Casting.ID} || ${Me.Class.ShortName.Equal[BRD]}) && ${Me.Combat} && ${Defined[Abilities2D]}) /call CombatAbilities
/if (${Debug} || ${Debug_Assists}) /echo <== check_AssistStatus -|
/RETURN

|------------------------------------------------|
|- Uses combat abilities, AAs, and disciplines.	-|
|- e3_casting is not used, for performance, as these are short recast low impact only
|------------------------------------------------|
SUB CombatAbilities
  /if (${Debug} || ${Debug_Assists}) /echo |- CombatAbilities ==>
  /declare i int local
  | Prereq/priorities
  /if (${Defined[doTaunt]}) {
    /if (${doTaunt} && ${Me.AbilityReady[Taunt]} && ${Me.PctAggro}< 100) /doability Taunt
  }
  /if (${Defined[smartTaunt]}) {
    /if (${smartTaunt}) {
      /if (${Bool[${Me.TargetOfTarget}]} && !${Select[${Me.TargetOfTarget.Class.ShortName},WAR,PAL,SHD]}) {
        /if (${Select[${Spawn[id ${Target.ID}].Type},${AcceptableTargetTypes}]}) {
          /if (${Me.AbilityReady[Taunt]}) {
            /bc Taunting ${Target.CleanName}: ${Me.TargetOfTarget.Class.ShortName}-${Me.TargetOfTarget} has aggro
            /doability Taunt
          |} else /if (${Me.CombatAbilityReady[Mock]}) {
          |  /bc Mock on ${Target.CleanName}: ${Me.TargetOfTarget.Class.ShortName}-${Me.TargetOfTarget} has aggro
          |  /doability Mock
          | } else /if (${Me.CombatAbilityReady[Bazu Bellow]}) {
          |  /bc Bazu Bellow on ${Target.CleanName}: ${Me.TargetOfTarget.Class.ShortName}-${Me.TargetOfTarget} has aggro
          |   /doability "Bazu Bellow"
          } else /if (${Me.AltAbilityReady[Divine Stun]}) {
            /bc Divine Stun on ${Target.CleanName}: ${Me.TargetOfTarget.Class.ShortName}-${Me.TargetOfTarget} has aggro
            /casting "Divine Stun"
          } else /if (${Me.SpellReady[Terror of Discord]}) {
            /bc Terror of Discord on ${Target.CleanName}: ${Me.TargetOfTarget.Class.ShortName}-${Me.TargetOfTarget} has aggro
            /casting "Terror of Discord"
          }
        }
      }
    }
  }
  /if (${Defined[AutoEvade]}) {
    /if (${AutoEvade}) /call AutoEvade
  }

  /for i 1 to ${Abilities2D.Size[1]}
    |skip the ability if current aggro is less than defined PctAggro
    /if (${Abilities2D[${i},${iPctAggro}]} > 0 && (${Me.PctAggro} < ${Abilities2D[${i},${iPctAggro}]})) {
      /if (${Debug} || ${Debug_Assists}) /echo skipping ${Abilities2D[${i},${iCastName}]}, current aggro ${Me.PctAggro} defined ${Abilities2D[${i},${iPctAggro}]} named ${Target.Named}
      /goto :skipAbility
    }
    /if (${Abilities2D[${i},${iCastType}].Equal[Ability]}) {
      /if (${Abilities2D[${i},${iCastName}].Equal[Bash]}) {
        /if (!${Select[${Me.Inventory[Offhand].Type},Shield]} && !${Me.AltAbility[2 Hand Bash]}) /goto :skipAbility
      }
      /if (${Me.AbilityReady[${Abilities2D[${i},${iCastName}]}]}) {
        /doability "${Abilities2D[${i},${iCastName}]}"
        /delay 2 !${Me.AbilityReady[${Abilities2D[${i},${iCastName}]}]}
      }
      |- AA, only those with 0 cast time can go here
    } else /if (${Abilities2D[${i},${iCastType}].Equal[AA]}) {
      /if (${Me.AltAbilityReady[${Abilities2D[${i},${iCastName}]}]}) {
        /casting "${Abilities2D[${i},${iCastName}]}" alt
        /delay 3 !${Me.AltAbilityReady[${Abilities2D[${i},${iCastName}]}]}
      }
      |- Discipline, no disc with a duration can go here
    } else /if (${Abilities2D[${i},${iCastType}].Equal[Disc]}) {
    |/echo car ${Me.CombatAbilityReady[${abilityName}]} ${Me.CombatAbility[${abilityName}]} ${abilityName} ${Me.CombatAbilityReady[${abilityName}]} ${Me.CombatAbilityReady[Backstab]}
      /if (${Me.CombatAbilityReady[${Abilities2D[${i},${iCastName}]}]}) {
        |/echo ${Abilities2D[${i},${iCastName}]} myend ${Me.PctEndurance} >= minend ${Abilities2D[${i},${iMinEnd}]} and endcost ${Spell[${Abilities2D[${i},${iCastName}]}].EnduranceCost} <= ${Me.Endurance}
        /if (${Me.PctEndurance} >= ${Abilities2D[${i},${iMinEnd}]} && ${Spell[${Abilities2D[${i},${iCastName}]}].EnduranceCost} <= ${Me.Endurance} && ${Abilities2D[${i},${iIfs}]}) {
          |/echo ready ${abilityName} ${Me.CombatAbilityReady[${abilityName}]}
          /disc ${Abilities2D[${i},${iCastName}]}
          /delay 3 !${Me.CombatAbilityReady[${Abilities2D[${i},${iCastName}]}]}
        }
      }
    }
  :skipAbility
  /next i

  /if (${Debug} || ${Debug_Assists}) /echo <== CombatAbilities -|
/RETURN

|------------------------------------------------------------|
|- Turns off attack, when a mob you're attacking enrages.	-|
|------------------------------------------------------------|
#event EnrageOn "#1# has become ENRAGED."
SUB Event_EnrageOn(line, RageOn)
  /if (${Target.ID} == ${Spawn[${RageOn}].ID}) /varset isEnraged TRUE
  /if (${Me.Pet.ID}) {
    /pet back off
    /echo ${Me.Name} is backing off pet for enrage
  } 
/RETURN
|------------------------------------------------|
|- Turns attack back on, after enrage is over.	-|
|------------------------------------------------|
#event EnrageOff "#1# is no longer enraged."
SUB Event_EnrageOff(line, RageOn)
  /if (${Target.ID} == ${Spawn[${RageOn}].ID}) /varset isEnraged FALSE
  /if (${Me.Pet.ID}) /pet attack
/RETURN

|------------------------------------------------|
|- Adjust Melee distance if too far away msg   	-|
|------------------------------------------------|
#EVENT getCloser "Your target is too far away, get closer!"
SUB EVENT_getCloser
	/if (${Assisting} && ${AllowControl}) {
    /doevents flush EVENT_getCloser
		/if (${AssistStickDistance} > 5) /varcalc AssistStickDistance ${AssistStickDistance} - 3
	}
/if (${Debug} || ${Debug_Assists}) /echo EVENT_getCloser -| AssistStickDistance has been set to [${AssistStickDistance}]
/RETURN

|------------------------------------------------------------|
|- dont use this, its dumb and expensive
|------------------------------------------------------------|
SUB check_AutoAssist
/if (${Debug} || ${Debug_Assists}) /echo |- check_AutoAssist ==>
	/if (!${Assisting}) {
		/if ((${Target.ID} && ${Me.Combat}) || (${Target.ID} && ${Spell[${Me.Casting.ID}].SpellType.Equal[Detrimental]})) {
			/if (${Target.PctHPs} < ${EngagePct} && ${Select[${Spawn[id ${Target.ID}].Type},${AcceptableTargetTypes}]}) {
			  /if ((!${Bool[${PauseTarget}]} || ${Target.ID} != ${PauseTarget}) && (!${Bool[${Me.Pet.ID}]} || ${Target.ID} != ${Me.Pet.ID})) {
          /if (${Debug} || ${Debug_Assists}) /echo I am attacking, calling Assist
          /assistme
        }
			}
    }
	}
/if (${Debug} || ${Debug_Assists}) /echo <== check_AutoAssist -|
/RETURN

|--------------------------------------------------------|
|- Casts direct damage spells on a specified target.	  -|
|--------------------------------------------------------|
SUB check_Nukes

/if (${Debug} || ${Debug_Assists}) /echo |- check_Nukes ==>
	/if (${Defined[Nukes2D]} && ${Assisting} && ${use_Nukes}) {
	  /declare castIndex int local
	  /declare s int local
    /declare m int local
    /declare assistToT int local
    /declare typeOfCast string local 
    /declare HaveGiftOfManaBuff bool False local
    /if (${Bool[${Me.Song[Gift of Mana].ID}]} || ${Bool[${Me.Song[Celestial Gift].ID}]} || ${Bool[${Me.Song[Celestial Boon].ID}]}) {
        /varset HaveGiftOfManaBuff True
    }

   /for s 1 to ${Nukes2D.Size[1]}
    
    /varset typeOfCast  
    
    /varset castIndex ${s}
    /if (${Debug} || ${Debug_Assists}) /echo ${s} ${castIndex} ${Nukes2D[${castIndex},${iCastName}]}

    

    |allows use of druid/mage DS, etc, in nukes based on spellset. casts on assistTargets target
    /if (${Nukes2D[${castIndex},${iSpellType}].Find[Beneficial]}) {
      /if (${Target.ID} != ${AssistTarget}) /call TrueTarget ${AssistTarget}
      /if (${Select[${Me.TargetOfTarget.Class.ShortName},WAR,PAL,SHD]}) {
        /if (${Spawn[pc ${Me.TargetOfTarget}].ID}) {
          /varset assistToT ${Me.TargetOfTarget.ID}
          /call check_Ready "Nukes2D" ${castIndex}
          /if (${c_Ready} && ${Nukes2D[${castIndex},${iIfs}]}) {
            /if (${check_Mana["Nukes2D",${castIndex}]}) {
              /if (${check_Distance[${assistToT},${Nukes2D[${s},${iMyRange}]}]}) {
                /call e3_Cast ${assistToT} "Nukes2D" "${castIndex}"
              }
            }
          }
        }
      }
    } else {

      |When an item delay is applied to an entry, check to see if the delay is still active
      |Check lower comment after the spell is cast for more information.
      /if (${Defined[nukeSpellRecast_${Nukes2D[${castIndex},${iSpellID}]}]}) {
        /if (${nukeSpellRecast_${Nukes2D[${castIndex},${iSpellID}]}}) {
          |/bc check_nuke: skipping nukes as its timer isn't met yet:${Nukes2D[${castIndex},${iCastName}]} time left:${nukeSpellRecast_${Nukes2D[${castIndex},${iSpellID}]}}
          /goto :skipCast
        }
      }
      | skip casting if a delay is defined and active
      /if (${Defined[castDelay${Nukes2D[${castIndex},${iCastID}]}]}) {
        /if (${castDelay${Nukes2D[${castIndex},${iCastID}]}}) {
          /if (${Debug} || ${Debug_Assists}) /echo skipping nukes, /delay from previous cast
          /return
        }
      }
      |if im burning and /noburn is set, skip the spell
      |if noburn and rotate is defined, then skip this spell and set lastSuccessfulCast to this spell, so the next spell can be used
      /if ((${use_QUICKBurns} || ${use_LONGBurns} || ${use_FULLBurns}) && ${Nukes2D[${castIndex},${iNoBurn}]}) {
        /if (${Nukes2D[${castIndex},${iRotate}]} && ${Nukes2D.Size[1]} > 1) {
          /if (${Debug} || ${Debug_Assists}) /echo setting lastSuccessfulCast to ${lastSuccessfulCast} because noburn and rotate defined
          /varset lastSuccessfulCast ${Nukes2D[${castIndex},${iCastName}]}
        } else {
          /if (${Debug} || ${Debug_Assists}) /echo skipping cast of ${Nukes2D[${castIndex},${iCastName}]}, i am burning and /NoBurn is set
          /goto :skipCast
        }
      }
      | if /rotate is defined, and the current array index was the last successful cast, skip to next index
    /if (${Nukes2D[${castIndex},${iCastName}].Equal[${lastSuccessfulCast}]} && ${Nukes2D[${castIndex},${iRotate}]} && ${Nukes2D.Size[1]} > 1) {
      /if (${Debug} || ${Debug_Assists}) /echo Possibly skipping cast of ${Nukes2D[${castIndex},${iCastName}]} /rotate is defined
      |need to check if any of our other spells can cast, if not, we can ignore this rotate
      /declare otherSpellsReady bool local
      /varset otherSpellsReady FALSE
      /declare currentCastName string local
      /varset currentCastName ${Nukes2D[${castIndex},${iCastName}]}
      /declare i int local
      /for i 1 to ${Nukes2D.Size[1]}
        |if not the currently casting spell, lets check if they are ready to cast.          
        /if (!${Nukes2D[${i},${iCastName}].Equal[${currentCastName}]}) {
          
          /call check_Ready "Nukes2D" ${i}
          |When an item delay is applied to an entry, check to see if the delay is still active
          |Check lower comment after the spell is cast for more information.
          /if (${Defined[nukeSpellRecast_${Nukes2D[${i},${iSpellID}]}]}) {
            /if (${nukeSpellRecast_${Nukes2D[${i},${iSpellID}]}}) {
              /varset c_Ready FALSE
            }
          }
          /if (${c_Ready} && ${Nukes2D[${i},${iIfs}]}) {
            /if (${Bool[${Nukes2D[${i},${iGiftOfMana}]}]}) {
              |its specified to use gift of mana, well do we have it?
              /if (${HaveGiftOfManaBuff}) {
                /varset otherSpellsReady TRUE
                /if (${Debug} || ${Debug_Assists}) /echo Another spell is defined as ready:${Nukes2D[${i},${iCastName}]}
                /break
              }
            } else {
              /varset otherSpellsReady TRUE
              /if (${Debug} || ${Debug_Assists}) /echo Another spell is defined as ready:${Nukes2D[${i},${iCastName}]}
              /break
            }
          }
        }
      /next i
   
    
      |if others spells are ready to cast, its ok to skip this.
        /if (${otherSpellsReady}) {
          /if (${Debug} || ${Debug_Assists}) /echo another spell is ready to cast
           
          /goto :skipCast
        } else {
          /if (${Debug} || ${Debug_Assists}) /echo no other spell ready, allowing the current rotate spell to cast.
        }  
      }

    | if i have Gift Of Mana find a spell with /GoM, that is ready
      /if (${HaveGiftOfManaBuff}) {
        /if (${Debug} || ${Debug_Assists}) /echo have gift of mana, selecting GOM spell flag
        /for m 1 to ${Nukes2D.Size[1]}
          /if (${Nukes2D[${m},${iGiftOfMana}]}) {
        
            /call check_Ready "Nukes2D" ${m}
            /if (${c_Ready}) {
              /if (${Debug} || ${Debug_Assists}) /echo found a ready spell for GOM
              /varset castIndex ${m}
              |/bc CheckNukes_GOMBUFF: resetting  lastSuccessfulCast 
              /varset lastSuccessfulCast 0
              /break
            } 
          }
        /next m
      }
      |if this is a spell with /GoM, yet we don't have GoM buff, skip this spell.
      /if (!${HaveGiftOfManaBuff} && ${Bool[${Nukes2D[${castIndex},${iGiftOfMana}]}]} ) {
          | we do not have gift of mana, make sure our current index is not a GOM spell, if so skip it
          /if (${Debug} || ${Debug_Assists}) /echo skipping gift of mana spell ${Nukes2D[${castIndex},${iCastName}]}
          /goto :skipcast
      }

      |if i have aggro and /noaggro is set, skip the spell
      /if (${Nukes2D[${castIndex},${iNoAggro}]}) {
        /if (${Me.TargetOfTarget.CleanName.Equal[${Me.CleanName}]}) {
          /if (${Debug} || ${Debug_Assists}) /echo skipping cast of ${Nukes2D[${castIndex},${iCastName}]}, i have aggro and /noaggro is set
          /goto :skipCast
        }
      }
      |only cast if my aggro is greater than PctAggro and the target is a Named
      /if (${Nukes2D[${castIndex},${iPctAggro}]} > 0) {
        /if (${Me.PctAggro} < ${Nukes2D[${castIndex},${iPctAggro}]} || !${Target.Named}) {
          /if (${Debug} || ${Debug_Assists}) /echo skipping cast of ${Nukes2D[${castIndex},${iCastName}]}, my aggro ${Me.PctAggro} is < PctAggro ${Nukes2D[${castIndex},${iPctAggro}]}
          /if (${Debug} || ${Debug_Assists}) /echo ${Nukes2D[${castIndex},${iPctAggro}]} ${Me.PctAggro} < ${Nukes2D[${castIndex},${iPctAggro}]} !${Target.Named}
          /goto :skipCast
        }
      }

      | if i have spell damage reduction recourse and the mob has more than 25% health or i have < 10% mana, skip cast.  cast anyway if i have GoM to avoid losing it
      /if (${Me.Buff[Weave of Weakness].ID} || ${Me.Buff[Flames of Weakness].ID}) {
        /if ((${Target.PctHPs} > 25 || ${Me.PctMana} < 10) && !${Bool[${Me.Song[Gift of Mana].ID}]} && !${use_QUICKBurns} && !${use_LONGBurns} && !${use_FULLBurns}) {
          /if (${Debug} || ${Debug_Assists}) /echo skipping cast of ${Nukes2D[${castIndex},${iCastName}]}, i have spell damage reduction
          /goto :skipCast
        }
      }

      | fix for using Ifs in Nukes, retarget to the AssistTarget before checking Ifs
  	  /if (${Target.ID} != ${AssistTarget}) /call TrueTarget ${AssistTarget}
	  
      /call check_Ready "Nukes2D" ${castIndex}
	  
      /if (${Debug} || ${Debug_Assists}) {
      /echo Checking Nuke ${Nukes2D[${castIndex},${iIfs}]}
      /echo Mana ${check_Mana["Nukes2D",${castIndex}]}
      /echo Target  ${AssistTarget}
      /echo Cast index ${castIndex}
      /echo Check Distance ${check_Distance[${AssistTarget},${Nukes2D[${s},${iMyRange}]}]}
      /echo Range ${Nukes2D[${s},${iMyRange}]}
      }
	  
      /if (${c_Ready} && ${Nukes2D[${castIndex},${iIfs}]}) {
        /if (${check_Mana["Nukes2D",${castIndex}]}) {
          /if (${check_Distance[${AssistTarget},${Nukes2D[${s},${iMyRange}]}]}) {

             /if (${Select[${Me.Class.ShortName},CLR,DRU,SHM]}) {
     
               /varset Nukes2D[${castIndex},${iSubToRun}] check_HealCasting_DuringDetrimental
           
            }
            /varset typeOfCast ${Nukes2D[${castIndex},${iCastType}]}
            /call e3_Cast ${AssistTarget} "Nukes2D" "${castIndex}"
          }
        }
      }
      

      /if (${Select[${castReturn},CAST_SUCCESS]} && ${typeOfCast.Equal[Item]}) {
          |when an item is click/cast, the server sends over the cooldown of said item
          |so for a breif time between clicking there is an 'unkown' state where the items
          |cooldown is still 0, even tho it isn't valid. Noticed this when using molten orbs.
          |Adding a small delay to any item that is cast to prevent it from being a valid target
          |on the next event cycle as event cycles in e3 are far faster than the server can send us
          |the cooldown information
          /call CreateTimer nukeSpellRecast_${Nukes2D[${castIndex},${iSpellID}]} "1s"
      }
      |if the nuke was succesful, see if there is a delay defined and create a timer which locks out all nukes for the duration
      /if (${Bool[${Nukes2D[${castIndex},${iDelay}]}]} && ${Select[${castReturn},CAST_SUCCESS]}) {
        /if (${Debug} || ${Debug_Assists}) /echo delaying ${Nukes2D[${castIndex},${iCastName}]} for ${Nukes2D[${castIndex},${iDelay}]}
        /call CreateTimer "castDelay${Nukes2D[${castIndex},${iCastID}]}" "${Nukes2D[${castIndex},${iDelay}]}"
      }
    }
	:skipCast
    /if (!${ActionTaken}) /next s
  }
/if (${Debug} || ${Debug_Assists}) /echo <== check_Nukes -|
/RETURN

|--------------------------------------------------------------------------------|
| Author: Sirhopsalot
| Casts spells on the non-assist target (but only while assist is active)
| this is useful for casting dots / debuffs on the non-assist targets 
| when you know the main assist target will be dead soon
|--------------------------------------------------------------------------------|
SUB check_OffAssistSpells
  | If we're not currently attacking something, don't do anything
  /if (!${Assisting}) /return
  | loop through the xtargets checking if any match our criteria
  | break on the first one that matches
  /declare pctAggro int local 80
  /declare minHp int local 10
  /declare highestHp int local 0
  /declare mobId int local 0
  /declare i int local
  /for i 1 to 13
    /if (${Bool[${Me.XTarget[${i}]}]} && ${Me.XTarget[${i}].TargetType.Equal[Auto Hater]} && ${Me.XTarget[${i}].Type.Equal[NPC]} && ${Me.XTarget[${i}].Aggressive} && ${Me.XTarget[${i}].ID} != ${AssistTarget} && ${Me.XTarget[${i}].PctHPs} >= ${minHp} && ${Me.XTarget[${i}].PctAggro} < ${pctAggro} && !${Bool[${Me.XTarget[${i}].Name.Find[Corpse]}]} && ${Me.XTarget[${i}].LineOfSight}) {
      /if (${Me.XTarget[${i}].PctHPs} > ${highestHp}) {
        /varset mobId ${Me.XTarget[${i}].ID}
        /varset highestHp ${Me.XTarget[${i}].PctHPs}
      }
    }
  /next i
  /if (${Bool[${mobId}]}) {
    /docommand ${ChatToggle} Casting offtarget spell on ${Spawn[id ${mobId}].Name}
    /call cast_longTermSpells "${mobId}" "OffAssist_Spells2D"
  }
/RETURN

|----------------------------------------------------|
|- interrupts casting if target is invalid (dead)   -|
|----------------------------------------------------|
|Sub check_detrimental_target
|/if (${Debug} || ${Debug_Assists}) /echo |- check_detrimental_target ==>
|  /delay 1
|/if (${Debug} || ${Debug_Assists}) /echo <== check_detrimental_target -|
|/RETURN ${Bool[${Spawn[id ${Target.ID}].Type.NotEqual[Corpse]}]}

|----------------------------------------------------|
|- Enables Debuffing on a Target                    -|
|----------------------------------------------------|
SUB check_Debuffs
  /if (${Defined[Command_Debuffs2D]}) /call Debuffs_OnCommand
  /if (${Defined[All_Debuffs2D]}) /call Debuffs_OnAll
	/if (${Assisting} && ${Defined[Assist_Debuffs2D]}) /call cast_longTermSpells "${AssistTarget}" "Assist_Debuffs2D"
/if (${Debug} || ${Debug_Assists}) /echo <== check_Debuffs -|
/RETURN

|----------------------------------------------------|
|- Enables Dots on a Target                         -|
|----------------------------------------------------|
SUB check_DoTs
/if (${Debug} || ${Debug_Assists}) /echo |- check_DoTs ==>
  /if (${Defined[Command_DoTs2D]} && ${use_CommandDots})  /call Dots_OnCommand
	/if (${Assisting} && ${Defined[Assist_DoTs2D]} && ${use_AssistDots}) /call cast_longTermSpells "${AssistTarget}" "Assist_DoTs2D"
/if (${Debug} || ${Debug_Assists}) /echo <== check_DoTs -|
/RETURN

|----------------------------------------------------|
|- Casts debuffs and dots on various assist targets -|
|----------------------------------------------------|
SUB cast_longTermSpells(int spellTarget, string ArrayName)
  /if (${Debug} || ${Debug_Assists}) /echo |- cast_longTermSpells - [${ArrayName}] ==>
|/varset Debug_Assists TRUE
  /declare spellTarg int local
  /declare spellTimer string local
  /declare spellRecast int local 0
  /declare s int local
  /declare checkForName string local

	/for s 1 to ${${ArrayName}.Size[1]}
    /varset checkForName	${${ArrayName}[${s},${iCheckFor}]}

	  |this check lets you use OOW debuff BP in debuff section
	  /if (${${ArrayName}[${s},${iSpellType}].Find[Beneficial]}) {
	    /varset spellTarg ${Me.ID}
      /if (${Debug} || ${Debug_Assists}) /echo ${${ArrayName}[${s},${iCastName}]} is beneficial, going directly to casting
	    /goto :attemptCast
    } else {
      /varset spellTarg ${spellTarget}
    }
		/varset spellTimer spellTimer_${spellTarg}_${ArrayName}_${s}
		/varset resistCount resist_${spellTarg}_${ArrayName}_${s}
    |/echo stdef ${Defined[${spellTimer}]} st ${spellTarg}  tgt ${Target.ID}
		/if (${Defined[${spellTimer}]}) {
		  /if (${${spellTimer}}) {
        /if (${Debug} || ${Debug_Assists}) /echo |- cast_longTermSpells - [${ArrayName}] -|	Waiting ${${spellTimer}} to recast ${${ArrayName}[${s},${iCastName}]} on ${spellTarg}.
        /goto :skipCast
      }
    }
    |check if the target is already debuffed, and if the duration is longer than the recast time
    /if (${spellTarg} != ${Target.ID}) {
      /call TrueTarget ${spellTarg}
      /delay 5 ${Target.BuffsPopulated}
    }
    /if (${Bool[${Select[${ArrayName},Command_Debuffs2D,Assist_Debuffs2D,All_Debuffs2D]}]}) {
      /if (${Target.BuffDuration[${${ArrayName}[${s},${iSpellName}]}]}/1000 > ${longTermRecastTime} && !${${ArrayName}[${s},${iSpellName}].Equal[${spammableSpell}]}) {
        /varcalc spellRecast ${Target.BuffDuration[${${ArrayName}[${s},${iSpellName}]}]}/1000 - ${longTermRecastTime}
        /call createTimer ${spellTimer} ${spellRecast}s
        /echo \ag ${Target.CleanName} already debuffed ${${ArrayName}[${s},${iSpellName}]} recast ${spellRecast}
        /goto :skipCast
      }

      /if (${Target.BuffDuration[${checkForName}]}/1000 > ${longTermRecastTime}) {
        /varcalc spellRecast ${Target.BuffDuration[${checkForName}]}/1000 - 10
        /call createTimer ${spellTimer} ${spellRecast}s
        /echo \ag ${Target.CleanName} already debuffed ${checkForName} recast ${spellRecast}
        /goto :skipCast
      }      
    }
    :attemptCast
    /if (!${${ArrayName}[${s},${iIfs}]}) {
        | need to have some sort of timer ... this will skip most encounters but raids will probably allow rechecking if conditions change
        /call createTimer "${spellTimer}" "30s"
        /if (${Debug} || ${Debug_Assists}) /echo \gy Skipping ${${ArrayName}[${s},${iSpellName}]} due to Ifs on ${Target.CleanName}
        /goto :skipCast
    }
    /call check_Ready "${ArrayName}" ${s}
    /if (${c_Ready}) {
      /if (${check_Mana[${ArrayName},${s}]}) {
        /if (${check_Distance[${spellTarg},${${ArrayName}[${s},${iMyRange}]}]}) {
    
            /if (${Select[${Me.Class.ShortName},CLR,DRU,SHM]}) {
               /varset ${ArrayName}[${s},${iSubToRun}] check_HealCasting_DuringDetrimental
            }
          /call e3_Cast ${spellTarg} "${ArrayName}" "${s}"
        } else {
          /if (${Debug} || ${Debug_Assists}) /echo ${spellTarg} out of range for ${${ArrayName}[${s},${iMyRange}]} ${${ArrayName}[${s},${iCastName}]}
          /goto :skipCast
        }
      } else {
        /if (${Debug} || ${Debug_Assists}) /echo lack mana for ${${ArrayName}[${s},${iCastName}]}
        /goto :skipCast
      }
    } else {
      /if (${Debug} || ${Debug_Assists}) /echo ${${ArrayName}[${s},${iCastName}]} not ready
      /goto :skipCast
    }
    |/if (${Debug} || ${Debug_Assists}) /echo castretun ${castReturn} res ${Cast.Result}
    |/if (${Debug} || ${Debug_Assists}) /echo maxtries ${${ArrayName}[${i},${iMaxTries}]} rescount ${resistCount} ${${resistCount}}
    |/if (${Debug} || ${Debug_Assists}) /echo castname ${${ArrayName}[${i},${iCastName}]} stored ${Cast.Stored.Name} return ${castReturn}
    | If the spell was resisted.	
    /if (${Cast.Stored.Name.Equal[${${ArrayName}[${s},${iSpellName}]}]} && ${castReturn.Equal[CAST_RESIST]}) {
      /if (!${Defined[${resistCount}]}) {
        /declare ${resistCount} int outer 1
      } else {
        /varcalc ${resistCount} ${${resistCount}}+1
        /if (${${resistCount}} >= ${${ArrayName}[${s},${iMaxTries}]}) {
          /deletevar ${resistCount}
          /call createTimer "${spellTimer}" "1m"
        }
      }
    | If the spell did not take hold.
    } else /if (${Cast.Stored.Name.Equal[${${ArrayName}[${s},${iSpellName}]}]} && ${castReturn.Equal[CAST_TAKEHOLD]} && !${Cast.Stored.Name.Equal[${spammableSpell}]}) {
      /if (${Defined[${resistCount}]}) {
        /deletevar ${resistCount}
      }
      /call createTimer "${spellTimer}" "3m"
      | If the target was immune.
    } else /if (${Cast.Stored.Name.Equal[${${ArrayName}[${s},${iSpellName}]}]} && ${castReturn.Equal[CAST_IMMUNE]} && !${Cast.Stored.Name.Equal[${spammableSpell}]}) {
      /if (${Defined[${resistCount}]}) {
        /deletevar ${resistCount}
      }
      /call createTimer "${spellTimer}" "30m"
    | If the cast was successful.
    } else /if (${Cast.Stored.Name.Equal[${${ArrayName}[${s},${iSpellName}]}]} && ${castReturn.Equal[CAST_SUCCESS]} && !${Cast.Stored.Name.Equal[${spammableSpell}]}) {
      /if (${Defined[${resistCount}]}) {
        /deletevar ${resistCount}
      }
      |if i dont have spell awarenes use default buff duration
      /varcalc spellRecast ${Target.BuffDuration[${${ArrayName}[${s},${iSpellName}]}]}/1000
      /if (${spellRecast}< ${Math.Calc[${${ArrayName}[${s},${iDuration}]}*6]}) /varcalc spellRecast ${${ArrayName}[${s},${iDuration}]}*6
      |/echo ${Target.Name}  ${Target.BuffDuration[${${ArrayName}[${i},${iSpellName}]}]} ${spellRecast}
      |duration over 90 seconds is considered long term
      /if (${spellRecast}>90) {
        /varcalc spellRecast ${spellRecast}-${longTermRecastTime}
      } else {
        /varcalc spellRecast ${spellRecast}-${shortTermRecastTime}
      }
      /if (${Debug} || ${Debug_Assists}) /echo |- longTermSpells - [${ArrayName}] -| Creating timer [${spellTimer}] -- [${spellRecast}]
      |/echo Creating timer [${spellTimer}] -- [${spellRecast}]
      /call createTimer "${spellTimer}" "${spellRecast}s"
    }
  :skipCast
  /if (!${ActionTaken}) /next s
/if (${Debug} || ${Debug_Assists}) /echo <== cast_longTermSpells - [${ArrayName}] -|
/RETURN

|---------------------------------------------------------------------------------------------|
|- Validates targets and adds them to a list of Debuff_Targets. Used for Debuffs_OnCommand.	 -|
|---------------------------------------------------------------------------------------------|
|- Use, '/Debuff' or 'Debuffs on ${Target.ID}' in /tell, /group, or /bc.					 -|
|---------------------------------------------------------------------------------------------|
#EVENT DebuffsOn "<#1#> Debuffs on #2#"
#EVENT DebuffsOn "#1# tells you, 'Debuffs on #2#'"
SUB Event_DebuffsOn(line, ChatSender, MobID)
/if (${Debug} || ${Debug_Assists}) /echo |- event_DebuffsOn ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${MobID}]}) /return
    /varset MobID ${c_eventArgData}
		/if (${Defined[Command_Debuffs2D]}) {
			| Check if the target is a corpse
			/if (${Spawn[id ${MobID}].Type.Equal[Corpse]} || !${Select[${Spawn[id ${MobID}].Type},${AcceptableTargetTypes}]}) {
        /docommand ${ChatToggle} Unacceptable Target ${Spawn[id ${MobID}].Type}
			} else {
				| Check range to the debuff target.
				/if (${Spawn[id ${MobID}].Distance} > ${MaxAssistDistance}) {
					/docommand ${ChatToggle} ${Spawn[${MobID}].CleanName} is too far away.
				} else {
					| Check if the Debuff_Targets array is already defined
					/if (${Defined[Debuff_Targets]}) {
						/declare i int local
						/for i 1 to ${Debuff_Targets.Size}
							/if (${Debuff_Targets[${i}].Arg[1,_]} == ${MobID}) {
								/docommand ${ChatToggle} I am already debuffing ${Spawn[id ${MobID}].CleanName}.
								/goto :Skip
							}
						/next i
					}
					/call BuildArray Debuff_Targets ${MobID}_${Zone.ID}
          |/docommand ${ChatToggle} -- Debuffing ${Spawn[${MobID}].CleanName} --
          /bc [+g+] -- Debuffing [${Spawn[${MobID}].CleanName}] --
				}
			}
		}
	:Skip
/if (${Debug} || ${Debug_Assists}) /echo <== event_DebuffsOn -|
/RETURN

|---------------------------------------------------------------------------------------------|
|- Validates targets and adds them to a list of Debuff_AllTargets. Used for Debuffs_OnAll.	 -|
|---------------------------------------------------------------------------------------------|
|- Use, '/Debuff' or 'Debuffs on ${Target.ID}' in /tell, /group, or /bc.					 -|
|---------------------------------------------------------------------------------------------|
#EVENT DebuffsAllOn "<#1#> Debuff all on #2#"
#EVENT DebuffsAllOn "#1# tells you, 'Debuff all on #2#'"
SUB Event_DebuffsAllOn(line, ChatSender, MobID)
/if (${Debug} || ${Debug_Assists}) /echo |- event_DebuffsOn ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${MobID}]}) /return
    /varset MobID ${c_eventArgData}
		/if (${Defined[All_Debuffs2D]}) {
			| Check if the target is a corpse
			/if (${Spawn[id ${MobID}].Type.Equal[Corpse]} || !${Select[${Spawn[id ${MobID}].Type},${AcceptableTargetTypes}]}) {
        /docommand ${ChatToggle} Unacceptable Target ${Spawn[id ${MobID}].Type}
			} else {
				| Check range to the debuff target.
				/if (${Spawn[id ${MobID}].Distance} > ${MaxAssistDistance}) {
					/docommand ${ChatToggle} ${Spawn[${MobID}].CleanName} is too far away.
				} else {
					| Check if the Debuff_AllTargets array is already defined
					/if (${Defined[Debuff_AllTargets]}) {
						/declare i int local
						/for i 1 to ${Debuff_AllTargets.Size}
							/if (${Debuff_AllTargets[${i}].Arg[1,_]} == ${MobID}) {
								/docommand ${ChatToggle} I am already debuffing ${Spawn[id ${MobID}].CleanName}.
								/goto :Skip
							}
						/next i
					}
					/call BuildArray Debuff_AllTargets ${MobID}_${Zone.ID}
          |/docommand ${ChatToggle} -- Debuffing ${Spawn[${MobID}].CleanName} --
          /bc [+g+] -- Debuffing [${Spawn[${MobID}].CleanName}] --
				}
			}
		}
	:Skip
/if (${Debug} || ${Debug_Assists}) /echo <== event_DebuffsOn -|
/RETURN

|------------------------------------------------|
|- Cancels debuffs on command on all targets. 	-|
|------------------------------------------------|
#EVENT DebuffsOff "<#1#> End debuffs"
#EVENT DebuffsOff "<#1#> End debuffs on #2#"
SUB event_DebuffsOff(line, ChatSender, MobID)
/if (${Debug} || ${Debug_Assists}) /echo |- event_DebuffsOff ==>
	/if (${Defined[Debuff_Targets]}) {
	  /if (!${Defined[MobID]}) /declare MobID int
    /if (!${checkEventArgs[${ChatSender},${line},UZR,${MobID}]}) /return
    /if (${Bool[${c_eventArgData}]}) {
      /docommand ${ChatToggle} Ending Debuffs on ${Spawn[id ${c_eventArgData}].CleanName}.
      /call RemoveArrayElement Debuff_Targets ${c_eventArgData}_${Zone.ID}
    } else {
      /docommand ${ChatToggle} Ending Debuffs on all targets.
      /deletevar Debuff_Targets
    }
  }
/if (${Debug} || ${Debug_Assists}) /echo <== event_DebuffsOff -|
/RETURN

|--------------------------------------------------------------------|
|- Casts Command_Debuffs on targets listed in DebuffTargets array.	-|
|--------------------------------------------------------------------|
SUB Debuffs_OnCommand
/if (${Debug} || ${Debug_Assists}) /echo |- Debuffs_OnCommand ==>
	/declare targetID int local
	/declare targetZone int local
	/declare i int local
	/if (${Defined[Debuff_Targets]}) {
    /for i 1 to ${Debuff_Targets.Size}
      /varset targetID ${Debuff_Targets[${i}].Arg[1,_]}
      /varset targetZone ${Debuff_Targets[${i}].Arg[2,_]}
      | Check Mob Zone
      /if (${Zone.ID} != ${targetZone}) {
        /call RemoveArrayElement Debuff_Targets ${Debuff_Targets[${i}]}
      } else {
        | If the Debuff_Target has died, remove them from the array.
        /if (!${Bool[${Spawn[id ${targetID}].Type.NotEqual[Corpse]}]}) {
          /call RemoveArrayElement Debuff_Targets ${Debuff_Targets[${i}]}
        } else {
          /call cast_longTermSpells "${targetID}" "Command_Debuffs2D"
        }
      }
      /if (!${ActionTaken} && ${Defined[Debuff_Targets]}) /next i
  }
/if (${Debug} || ${Debug_Assists}) /echo <== Debuffs_OnCommand -|
/RETURN

|--------------------------------------------------------------------|
|- Casts All_Debuffs on targets listed in Debuff_AllTargets array.	-|
|--------------------------------------------------------------------|
SUB Debuffs_OnAll
/if (${Debug} || ${Debug_Assists}) /echo |- Debuffs_OnAll ==>
	/declare targetID int local
	/declare targetZone int local
	/declare i int local
	/if (${Defined[Debuff_AllTargets]}) {
    /for i 1 to ${Debuff_AllTargets.Size}
      /varset targetID ${Debuff_AllTargets[${i}].Arg[1,_]}
      /varset targetZone ${Debuff_AllTargets[${i}].Arg[2,_]}
      | Check Mob Zone
      /if (${Zone.ID} != ${targetZone}) {
        /call RemoveArrayElement Debuff_AllTargets ${Debuff_AllTargets[${i}]}
      } else {
        | If the Debuff_Target has died, remove them from the array.
        /if (!${Bool[${Spawn[id ${targetID}].Type.NotEqual[Corpse]}]}) {
          /call RemoveArrayElement Debuff_AllTargets ${Debuff_AllTargets[${i}]}
        } else {
          /call cast_longTermSpells "${targetID}" "All_Debuffs2D"
        }
      }
      /if (!${ActionTaken} && ${Defined[Debuff_AllTargets]}) /next i
  }
/if (${Debug} || ${Debug_Assists}) /echo <== Debuffs_OnAll -|
/RETURN

|---------------------------------------------------------------------------------------------|
|- Validates targets and adds them to a list of Debuff_Targets. Used for Debuffs_OnCommand.	 -|
|---------------------------------------------------------------------------------------------|
|- Use, '/DoTs' or 'DoTs on ${Target.ID}' in /tell or /bc.							 -|
|---------------------------------------------------------------------------------------------|
#EVENT DoTsOn "<#1#> DoTs on #2#"
#EVENT DoTsOn "#1# tells you, 'DoTs on #2#'"
SUB Event_DoTsOn(line, ChatSender, MobID)
/if (${Debug} || ${Debug_Assists}) /echo |- event_DoTsOn ==>
  /if (${Defined[Command_DoTs2D]}) {
    /if (!${checkEventArgs[${ChatSender},${line},UZR,${MobID}]}) {
		/echo event args failed
		/return
	}
      /varset MobID ${c_eventArgData}
      /if (${Spawn[id ${MobID}].Type.Equal[Corpse]} || !${Select[${Spawn[id ${MobID}].Type},${AcceptableTargetTypes}]}) {
        /echo Unacceptable Target ${Spawn[id ${MobID}].Type}
      } else {
        | Check range to the DoTs target.
        /if (${Spawn[id ${MobID}].Distance} > ${MaxAssistDistance}) {
			/echo ${Spawn[${MobID}].CleanName} is too far away
          /docommand ${ChatToggle} ${Spawn[${MobID}].CleanName} is too far away.
        } else {
          | Check if the DoT_Targets array is already defined
          /if (${Defined[DoT_Targets]}) {
            /declare i int local
            /for i 1 to ${DoT_Targets.Size}
              /if (${DoT_Targets[${i}].Arg[1,_]} == ${MobID}) {
                /docommand ${ChatToggle} I am already dotting ${Spawn[id ${MobID}].CleanName}.
                /goto :Skip
              }
            /next i
          }
          /call BuildArray DoT_Targets ${MobID}_${Zone.ID}
        }
      }
		}
	:Skip
/if (${Debug} || ${Debug_Assists}) /echo <== event_DoTsOn -|
/RETURN

|------------------------------------------------|
|- Cancels debuffs on command on all targets. 	-|
|------------------------------------------------|
#EVENT DoTsOff "<#1#> End DoTs"
#EVENT DoTsOff "<#1#> End DoTs on #2#"
SUB event_DoTsOff(line, ChatSender, MobID)
/if (${Debug} || ${Debug_Assists}) /echo |- event_DoTsOff ==>
	/if (${Defined[DoT_Targets]}) {
	/if (!${Defined[MobID]}) /declare MobID int local
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${MobID}]}) /return
  /if (${Bool[${c_eventArgData}]}) {
    /docommand ${ChatToggle} Ending DoTs on ${Spawn[id ${c_eventArgData}].CleanName}.
    /call RemoveArrayElement DoT_Targets ${c_eventArgData}_${Zone.ID}
  } else {
    /docommand ${ChatToggle} Ending DoTs on all targets.
    /deletevar DoT_Targets
  }
  }
/if (${Debug} || ${Debug_Assists}) /echo <== event_DoTsOff -|
/RETURN

|----------------------------------------------------------------|
|- Casts Command_DoTs on targets listed in DoT_Targets array.	-|
|----------------------------------------------------------------|
SUB DoTs_OnCommand
/if (${Debug} || ${Debug_Assists}) /echo |- DoTs_OnCommand ==>
	/declare targetID int local
	/declare targetZone int local
	/declare i int local
	/if (${Defined[DoT_Targets]}) {
    /for i 1 to ${DoT_Targets.Size}
      /varset targetID ${DoT_Targets[${i}].Arg[1,_]}
      /varset targetZone ${DoT_Targets[${i}].Arg[2,_]}
      | Check Mob Zone
      /if (${Zone.ID} != ${targetZone}) {
        /call RemoveArrayElement DoT_Targets ${DoT_Targets[${i}]}
      } else {
        | If the DoT_Target has died, remove them from the array.
        /if (!${Bool[${Spawn[id ${targetID}].Type.NotEqual[Corpse]}]}) {
          /call RemoveArrayElement DoT_Targets ${DoT_Targets[${i}]}
        } else {
          |/call cast_longTermSpells "${targetID}" "Command_DoTs2D"
          |Ewiclip MOD: New method for DoTs
          /call cast_damageOverTimeSpells ${targetID} "Command_DoTs2D"
        }
      }
    /if (!${ActionTaken} && ${Defined[DoT_Targets]}) /next i
  }
/if (${Debug} || ${Debug_Assists}) /echo <== DoT_OnCommand -|
/RETURN

|--------------------------------------|
| -- Ewiclip MOD: New Method for DoTs -|
|--------------------------------------|

SUB cast_damageOverTimeSpells(int TargetID, string ArrayName)

/declare i int local
/declare iSpellSwap int local 0
/declare spellTimer string local
|${${ArrayName}[${i},${iCastName}]}        --> strSpellName
|${${ArrayName}[${i},${iAllowSpellSwap}]} --> 0/1 
|${${ArrayName}[${i},${iNoEarlyRecast}]}  --> 0/1
|${${ArrayName}[${i},${iNoStack}]}        --> 0/1
|${${ArrayName}[${i},${iTriggerSpell}]}   --> 0/strSpellName
|${${ArrayName}[${i},${iIfs}]}            --> /noaprse string
|${${ArrayName}[${i},${iMinMana}]}        --> int 
|${${ArrayName}[${i},${iSpellGem}]}        --> int 

/for i 1 to ${${ArrayName}.Size[1]}
    /doevents DoTsOff
    |make sure we have the right target    
   
    |make sure our target is not a corpse
    /if (!${Bool[${Spawn[id ${TargetID}].Type.NotEqual[Corpse]}]}) {
      /return
    }

    |setups for resist counters and spellRetry timers
    /varset spellTimer spellTimer_${TargetID}_${ArrayName}_${i}
		/varset resistCount resist_${TargetID}_${ArrayName}_${i}
    |Is there a spell timer configured for this spell and target? if so, skip this   
    /if (${Defined[${spellTimer}]}) {
		  /if (${${spellTimer}}) {
        |/echo skipping ${spellTimer}
        /continue
      }
    }
    | -- Rules that tell us to skip the DoT we're on and evaluate off the next DoT.
    |Skip if /MinMana|%% is set and my mana percentage is below the defined amount in the parameter
    /if (!${${ArrayName}[${i},${iMinMana}]} > ${Me.PctMana}) {
      /continue
    }
    |Skip of defiend IF isn't met
    /if (!${${ArrayName}[${i},${iIfs}]}) {
      /continue
    }
    |verify the spell we want to use is actulaly ready
    /if (!(${Me.ItemReady[${${ArrayName}[${i},${iCastName}]}]} || ${Me.SpellReady[${${ArrayName}[${i},${iCastName}]}]} || ${Me.AltAbilityReady[${${ArrayName}[${i},${iCastName}]}]})) {
       /continue
    }
    |Change the target here, as we need the correct target if using TimeLeftOnMySpell
    /if (${Bool[${Target.ID} != ${TargetID}]}) {
      /if (${Debug} || ${Debug_Assists})  /echo changing target to ${TargetID}
      /target id ${TargetID}
      /delay 2
    }
    |Skip if /NoEarlyRecast is set and the mob has this spell from me
    /if (${TimeLeftOnMySpell[${${ArrayName}[${i},${iCastName}]}]} > 0 && ${${ArrayName}[${i},${iNoEarlyRecast}]}) {
      /continue
    }
    |Skip if /NoStack is set and ANYONE (including myself) have already casted this on the NPC
    /if (${${ArrayName}[${i},${iNoStack}]} && (${Bool[${Target.Buff[${FindItem[=${${ArrayName}[${i},${iCastName}]}].Spell}]}]} || ${Bool[${Target.Buff[=${${ArrayName}[${i},${iCastName}]}]}]})) {
      /continue
    }
    |Skip if /TriggerSpell|SpellName is set and both the spell and TriggerSpell are already on the NPC from me
    /if (${Bool[${${ArrayName}[${i},${iTriggerSpell}]}]} && ${TimeLeftOnMySpell[${${ArrayName}[${i},${iTriggerSpell}]}]} > 4 && ${TimeLeftOnMySpell[${${ArrayName}[${i},${iCastName}]}]} > 4) {
      /continue
    }
    |Skip if this spell is already on the NPC from me and has more than 4 seconds left on it. (This is item casts)
    /if (${Bool[${FindItem[=${${ArrayName}[${i},${iCastName}]}]}]} && ${TimeLeftOnMySpell[${FindItem[=${${ArrayName}[${i},${iCastName}]}].Spell}]} > 4) {
      /continue
    }
    |Skip if this spell is already on the NPC from me and has more than 4 seconds left on it. (This is spell casts)
    /if (${TimeLeftOnMySpell[${${ArrayName}[${i},${iCastName}]}]} > 4 && !${Bool[${${ArrayName}[${i},${iTriggerSpell}]}]}) {
      /continue
    }
 

    /if (!${Bool[${Me.Casting}]}) {

     
      |Cast the spell!
      /call e3_Cast ${TargetID} ${ArrayName} ${i}
      
      /if (${Cast.Stored.Name.Equal[${${ArrayName}[${i},${iSpellName}]}]} && ${castReturn.Equal[CAST_RESIST]}) {
       
        /if (!${Defined[${resistCount}]}) {
          |this will declare a resist counter as an int outer 1, so we can also keep track of it to delete it later.
          /call register_ResistCounter ${resistCount}
        
        } else {
          /varcalc ${resistCount} ${${resistCount}}+1
          /if (${Debug} || ${Debug_Assists})  /echo incrementing resist counter to:${${resistCount}} for:${resistCount}
          /if (${${resistCount}} >= ${${ArrayName}[${i},${iMaxTries}]}) {
            /varset ${resistCount} 0
            /call createTimer "${spellTimer}" "1m"
        }
      }
      | If the spell did not take hold.
      } else /if (${Cast.Stored.Name.Equal[${${ArrayName}[${i},${iSpellName}]}]} && ${castReturn.Equal[CAST_TAKEHOLD]} && !${Cast.Stored.Name.Equal[${spammableSpell}]}) {
        /if (${Defined[${resistCount}]}) {
           /varset ${resistCount} 0
        }
        |/echo creating spell timer for 3 min before retry
        /call createTimer "${spellTimer}" "3m"
        | If the target was immune.
      } else /if (${Cast.Stored.Name.Equal[${${ArrayName}[${i},${iSpellName}]}]} && ${castReturn.Equal[CAST_IMMUNE]} && !${Cast.Stored.Name.Equal[${spammableSpell}]}) {
        /if (${Defined[${resistCount}]}) {
           /varset ${resistCount} 0
        }
        /call createTimer "${spellTimer}" "5m"
      }
      |Spell Swap Code Specifically for /AllowSpellSwap
      |These spells will De-Mem and Re-Mem a new spell immediately after cast. /Gem|# is ignored for these spells. 
      /if (${Bool[${Me.SpellInCooldown}]} && ${${ArrayName}[${i},${iAllowSpellSwap}]} && ${Bool[${Me.Book[${${ArrayName}[${i},${iCastName}]}]}]}) {
        /for iSpellSwap 1 to ${${ArrayName}.Size[1]}
          /if (${TimeLeftOnMySpell[${${ArrayName}[${iSpellSwap},${iCastName}]}]} > 18 || ${Bool[${Me.Gem[${${ArrayName}[${iSpellSwap},${iCastName}]}]}]} || !${Bool[${Me.Book[${${ArrayName}[${iSpellSwap},${iCastName}]}]}]} || !${${ArrayName}[${iSpellSwap},${iAllowSpellSwap}]}) {
            /continue
          }
              |After /MemSpell the book will automatically close. However, while in combat this command sometimes fails to start the first time we send it.
              |We put this into a loop to ensure "it as least tries" -- If the spell really does fail to mem it will exit on it's own and try again next cycle.
              |/MemSpell {int WhereToMem} {String SpellToMem}
              /while (!${Window[SpellBookWnd].Open}) {
                /if (${GetEmptySpellGem[]} > 0) {
                  /MemSpell ${GetEmptySpellGem[]} "${${ArrayName}[${iSpellSwap},${iCastName}]}" 
                } else {
                  /MemSpell ${Me.Gem[${${ArrayName}[${i},${iCastName}]}]} "${${ArrayName}[${iSpellSwap},${iCastName}]}" 
                }
              }
              /break
         /next iSpellSwap
      } else {
        |Code to mem any spell in any empty gem with /Gem|# as the preferred location. This only applies to spells that DO NOT have /AllowSpellSwap defined.
        |/AllowSpellSwap spells are mem-shuffled with the code directly above this. And we ignore /Gem|# for those. 
        |In E3, if no /Gem|# is defined it automatically assigns the ${DefaultGem} to that specific spell's ${iSpellGem} variable.
        /if (${Bool[${Me.SpellInCooldown}]} && ${Bool[${Me.Book[${${ArrayName}[${i},${iCastName}]}]}]} && !${Bool[${${ArrayName}[${i},${iSpellSwap}]}]}) {
          /for iSpellSwap 1 to ${${ArrayName}.Size[1]}
            /if (!${Bool[${Me.Gem[${${ArrayName}[${iSpellSwap},${iCastName}]}]}]} && ${Bool[${Me.Book[${${ArrayName}[${iSpellSwap},${iCastName}]}]}]}) {
              |After /MemSpell the book will automatically close. However, while in combat this command sometimes fails to start the first time we send it.
              |We put this into a loop to ensure "it as least tries" -- If the spell really does fail to mem it will exit on it's own and try again next cycle.
              |/MemSpell {int WhereToMem} {String SpellToMem}
              /while (!${Window[SpellBookWnd].Open}) {
                |If I join this IF with an else /if it causes a hard client crash. --- /break prevents it from sending 2 different MemSpells
                /if (${${ArrayName}[${iSpellSwap},${iSpellGem}]} != ${DefaultGem}) {
                  /MemSpell ${${ArrayName}[${iSpellSwap},${iSpellGem}]} "${${ArrayName}[${iSpellSwap},${iCastName}]}"
                  /break
                }
                /if (${GetEmptySpellGem[]} > 0) {
                  /MemSpell ${GetEmptySpellGem[]} "${${ArrayName}[${iSpellSwap},${iCastName}]}" 
                } else {
                  /MemSpell ${DefaultGem} "${${ArrayName}[${iSpellSwap},${iCastName}]}" 
                }
              }
              /break
            }
          /next iSpellSwap
        }
      }
      |End Spell Swap Code

      |These checks are the same as above--but for the NEXT spell in the DoT array. This is so we don't QuickCast a spell that wouldn't be casted already.
      /if (${i} < ${${ArrayName}.Size[1]} && ${Bool[${Me.Book[${${ArrayName}[${i},${iCastName}]}]}]} && !${Bool[${Me.Book[${${ArrayName}[${Math.Calc[${i}+1]},${iCastName}]}]}]}) {
        
        |Incrementing loop up 1 so we can check the next spell in sequence. 
        /varcalc i ${i}+1
        |Skip if /MinMana|%% is set and my mana percentage is below the defined amount in the parameter
        /if (!${${ArrayName}[${i},${iMinMana}]} > ${Me.PctMana}) {
          /continue
        }

        |Skip if defiend IF isn't met
        /if (!${Bool[${${ArrayName}[${i},${iIfs}]}]}) {
          /return
        } 

        |Skip if /NoEarlyRecast is set and the mob has this spell from me
        /if (${TimeLeftOnMySpell[${${ArrayName}[${i},${iCastName}]}]} > 0 && ${${ArrayName}[${i},${iNoEarlyRecast}]}) {
          /return
        }

        |Skip if /NoStack is set and ANYONE (including myself) have already casted this on the NPC
        /if (${${ArrayName}[${i},${iNoStack}]} && (${Bool[${Target.Buff[${FindItem[=${${ArrayName}[${i},${iCastName}]}].Spell}]}]} || ${Bool[${Target.Buff[=${${ArrayName}[${i},${iCastName}]}]}]})) {
          /return
        }

        |Skip if /TriggerSpell|SpellName is set and both the spell and TriggerSpell are already on the NPC from me
        /if (${Bool[${${ArrayName}[${i},${iTriggerSpell}]}]} && ${TimeLeftOnMySpell[${${ArrayName}[${i},${iTriggerSpell}]}]} > 4 && ${TimeLeftOnMySpell[${${ArrayName}[${i},${iCastName}]}]} > 4) {
          /return
        }

        |Skip if this spell is already on the NPC from me and has more than 4 seconds let on it. (This is item casts and not spells)
        /if (${Bool[${FindItem[=${${ArrayName}[${i},${iCastName}]}]}]} && ${TimeLeftOnMySpell[${FindItem[=${${ArrayName}[${i},${iCastName}]}].Spell}]} > 4) {
          /return
        }

        |Skip if this spell is already on the NPC from me and has more than 4 seconds let on it. (This is spell casts and not items)
        /if (${TimeLeftOnMySpell[${${ArrayName}[${i},${iCastName}]}]} > 4 && !${Bool[${${ArrayName}[${i},${iTriggerSpell}]}]}) {
          /return
        }
        /echo QuickCast:: ${${ArrayName}[${i},${iCastName}]}
        /call e3_Cast ${TargetID} ${ArrayName} ${i} True
      } else {
        /return
      }
    }

/next i

/RETURN




|----------------------------|
|- Turns AE functions on.	  -|
|----------------------------|
#event AEs "<#1#> #2#AE #3#"
#event AEs "#1# tells you, '#2#AE #3#'"
SUB Event_AEs(line, ChatSender, aeType, aeToggle)
/if (${Debug} || ${Debug_Assists}) /echo |- Event_AllAEOn ==>
  /if (!${Select[${aeType},Target,PB,All]}) /return
  /if (${aeType.Equal[Target]} && !${Defined[TargetAE_Spells2D]})  /return
  /if (${aeType.Equal[PB]} && !${Defined[PBAE_Spells2D]})          /return
  /if (${aeType.Equal[All]} && (!${Defined[PBAE_Spells2D]} && !${Defined[TargetAE_Spells2D]})) /return

  /if (!${checkEventArgs[${ChatSender},${line},UZR,${aeToggle}]}) /return
  /varset aeToggle ${c_eventArgData}

  /if (${aeToggle.Equal[on]}) {
    /bc [+g+] [${aeType}AE]-${aeToggle.Upper}
    /if (${aeType.Equal[PB]} || ${aeType.Equal[All]}) {
      /varset use_PBAE TRUE
      /call TrueTarget ${NetBots[${c_eventArgChatSender}].ID}
      /call MoveToLoc ${Target.Y} ${Target.X} 30 18
    }
    /if (${aeType.Equal[Target]} || ${aeType.Equal[All]}) {
      /varset use_TargetAE TRUE
	  /if (${Debug} || ${Debug_Assists}) /echo use_TargetAE = TRUE
      /varset aeTarget ${Spawn[=${ChatSender}].ID}
      /if (${Me.Class.ShortName.Equal[RNG]}) {
        /if (${swapTgtAeRanged}) /call swapToRangedSet TRUE
      }
    }
  } else /if (${aeToggle.Equal[off]}) {
    /if (${use_PBAE} || ${use_TargetAE}) {
      /if (${Me.Casting.ID}) /call interrupt
      /bc [+y+] [${aeType}AE]-${aeToggle.Upper}
    }
    /if (${aeType.Equal[PB]} || ${aeType.Equal[All]}) {
      /varset use_PBAE FALSE
    }
    /if (${aeType.Equal[Target]} || ${aeType.Equal[All]}) {
      /varset use_TargetAE FALSE
      /varset aeTarget 0
      /varset aeTarget ${Spawn[=${ChatSender}].ID}
      /if (${revertRngToMelee}) /call swapToRangedSet FALSE
    }
  }
/if (${Debug} || ${Debug_Assists}) /echo <== Event_AllAEOn -|
/RETURN

|------------------------------------------------------------------------------------------------------------|
|- Casts spells listed in the [AE] section of the Character_Ini on mobs which wander into your AE_Radius.	-|
|------------------------------------------------------------------------------------------------------------|
SUB check_AE
/if (${Debug} || ${Debug_Assists}) /echo |- check_AE ==>
  /if ((${use_PBAE} || ${use_TargetAE}) && !${Bool[${SpawnCount[npc radius ${AE_Radius}]}]} && ${Me.CombatState.NotEqual[COMBAT]}) {
    /varset use_TargetAE FALSE
    /varset use_PBAE FALSE
    /varset aeTarget 0
    /docommand ${ChatToggle} Ending AE: No mobs in radius ${AE_Radius}
    /if (${revertRngToMelee}) /call swapToRangedSet FALSE
  }
  /declare s int local
	/if (${use_PBAE} && ${Defined[PBAE_Spells2D]}) {
    /for s 1 to ${PBAE_Spells2D.Size[1]}
      /call check_Ready "PBAE_Spells2D" ${s}
      /if (${c_Ready} && ${PBAE_Spells2D[${s},${iIfs}]}) {
        /if (${check_Mana["PBAE_Spells2D",${s}]}) {
          /squelch /face fast id ${AssistTarget}
          /call e3_Cast ${aeTarget} "PBAE_Spells2D" "${s}"
          /if (${Cast.Stored.Name.Equal[${PBAE_Spells2D[${s},${iSpellName}]}]} && ${castReturn.Equal[CAST_SUCCESS]}) {
            /if (${PBAE_Spells2D[${s},${iSpellName}].Equal[Idol of Malos]}) {
              /call CreateTimer "nht${aeTarget}-${PBAE_Spells2D[${s},${iCastID}]}" "1400"
            }
          }
        }
      }
    /if (!${ActionTaken}) /next s
	}
  /if (${use_TargetAE} && ${Defined[TargetAE_Spells2D]}) {
    |if i have an assist target, use that as ae target, else use aeTarget
    /declare useBestTarget int local
	/if (${Debug} || ${Debug_Assists}) /echo AssistTarget set = ${Bool[${AssistTarget}]}
    /if (${Bool[${AssistTarget}]}) {
      /varset useBestTarget ${AssistTarget}
    } else {
      /varset useBestTarget ${aeTarget}
    }
    /for s 1 to ${TargetAE_Spells2D.Size[1]}
      /call check_Ready "TargetAE_Spells2D" ${s}
      /if (${c_Ready} && ${TargetAE_Spells2D[${s},${iIfs}]}) {
        /if (${check_Mana["TargetAE_Spells2D",${s}]}) {
		  /if (${Debug} || ${Debug_Assists}) /echo Distance Check = ${check_Distance[${useBestTarget},${TargetAE_Spells2D[${s},${iMyRange}]}]}
		  /if (${Debug} || ${Debug_Assists}) /echo Range = ${TargetAE_Spells2D[${s},${iMyRange}]}
          /if (${check_Distance[${useBestTarget},${TargetAE_Spells2D[${s},${iMyRange}]}]}) {
            /squelch /face fast id ${useBestTarget}
            /call e3_Cast ${useBestTarget} "TargetAE_Spells2D" "${s}"
          }
        }
      }
    /if (!${ActionTaken}) /next s
  }
/if (${Debug} || ${Debug_Assists}) /echo <== check_AE -|
/RETURN

|----------------------------------|
|- Swap to Ranged for Target AE	  -|
|----------------------------------|
SUB swapToRangedSet(bool toggleRanged)
  /echo ${toggleRanged} toggleRanged
  /if (${toggleRanged} && ${AssistType.Equal[melee]}) {
    /varset AssistType Ranged
    /varset revertRngToMelee TRUE
    /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange" "${Me.Inventory[Ranged].Name}/Ranged" 1
    /delay 2
    /swap ranged
    /delay 2
  } else {
    /varset AssistType Melee
    /varset revertRngToMelee FALSE
    /swap melee
    /delay 2
    /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange" "NONE"
    /delay 2
  }
/RETURN

|----------------------------------------------------------------|
|- Casts swarm pets from Mages, Necros, Shaman, and Wizards.	-|
|----------------------------------------------------------------|
#event SwarmPets "<#1#> Swarm Pets on #2#"
SUB event_SwarmPets(line, ChatSender, int swarmTarget)
/if (${Debug} || ${Debug_Assists}) /echo |- event_SwarmPets ==>
  /if (${Defined[swarmPets2D]}) {
    /if (!${checkEventArgs[${ChatSender},${line},UZR,${swarmTarget}]}) /return
    /varset swarmTarget ${c_eventArgData}
		/if (${SpawnCount[id ${swarmTarget}]} && ${Spawn[${swarmTarget}].Type.NotEqual[Corpse]} && ${Select[${Spawn[${swarmTarget}].Type},${AcceptableTargetTypes}]}) {
			/declare i int local
			/for i 1 to ${swarmPets2D.Size[1]}
        /call check_Ready "swarmPets2D" ${i}
        /if (${c_Ready} && ${swarmPets2D[${i},${iIfs}]}) {
          /if (${check_Distance[${swarmTarget},${swarmPets2D[1,${iMyRange}]}]}) {
            /g SWARM-${swarmPets2D[${i},${iSpellName}]}
				    /call e3_Cast ${swarmTarget} "swarmPets2D" "${i}" 1
          }
        }
			/next i
		}
	}
/if (${Debug} || ${Debug_Assists}) /echo <== event_SwarmPets -|
/RETURN

|----------------------------------------------------|
SUB check_Burns
  /if (${Assisting}) {
    /if (${use_FULLBurns})  /call useBurns "FULLBurns2D"
    /if (${use_LONGBurns})  /call useBurns "LONGBurns2D"
    /if (${use_QUICKBurns}) /call useBurns "QUICKBurns2D"
    /if (${use_EPICBurns})  /call useBurns "EPICBurns2D"
  }
/RETURN

|----------------------------------------------------|
#EVENT Burns "<#1#> #2# Burn"
#EVENT Burns "#1# tells you, '#2# Burn'"
#EVENT Burns "#1# tells the group, '#2# Burn'"
#EVENT Burns "#1# tell your party, '#2# Burn'"
SUB event_Burns(line, ChatSender, burnType)
/if (${Debug} || ${Debug_Assists}) /echo |- event_Burns ==>
  /if (${line.Find[,]}) /varset line ${line.Left[${Math.Calc[${line.Find[,]}-1]}]} ${line.Right[-${line.Find[,]}]}
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${burnType}]}) /return
  /varset burnType ${c_eventArgData.Upper}
  /if (!${Assisting}) {
    /echo You must be attacking something to ${burnType} burn.
    /return
  }
  /if (!${Defined[${burnType}Burns2D]}) {
    /return
  } else {
    /if (${c_eventArgChatSender.Equal[${Me}]}) /bc [+g+] ${burnType} burns enabled
    /varset use_${burnType}Burns TRUE
  }
/return

|--------------------------------------------------------------------------------|
|- Casts spells, abilities, and burns through disciplines on a given targetID.	-|
|--------------------------------------------------------------------------------|
SUB useBurns(ArrayName)
/if (${Debug} || ${Debug_Assists}) /echo |- useBurns Array=${ArrayName} ==>
	/declare i int local
	/for i 1 to ${${ArrayName}.Size[1]}

    /call check_Ready "${ArrayName}" ${i}
    |/echo ${i} ${${ArrayName}[${i},${iCastName}]} tgt ${AssistTarget} rng ${${ArrayName}[${i},${iMyRange}]}
    |/echo ${i} rdy ${c_Ready} dist ${check_Distance[${AssistTarget},${${ArrayName}[${i},${iMyRange}]}]}
    /if (${c_Ready} && ${${ArrayName}[${i},${iIfs}]}) {
      /if (${check_Distance[${AssistTarget},${${ArrayName}[${i},${iMyRange}]}]}) {
        /if (${Me.ActiveDisc.ID} && ${${ArrayName}[${i},${iCastName}].Equal[Savage Spirit]}) /goto :skipCurrent
        /if (${Me.ActiveDisc.ID} && ${${ArrayName}[${i},${iCastType}].Equal[Disc]} && ${Bool[${${ArrayName}[${i},${iDuration}]}]}) /goto :skipCurrent
        /call e3_Cast ${AssistTarget} "${ArrayName}" "${i}"
        /delay 2
        |/varset ActionTaken TRUE
        /call check_Ready "${ArrayName}" ${i}
        /if (!${c_Ready}) /g ${ArrayName}-${${ArrayName}[${i},${iCastName}]}
      }
    } else {
      /if (${Debug} || ${Debug_Assists}) /echo NotReady: ${i} ${${ArrayName}[${i},${iCastName}]}
    }
  :skipCurrent
  |/delay 1
  /if (!${ActionTaken}) /next i
/if (${Debug} || ${Debug_Assists}) /echo <== useBurns -|
/RETURN

|--------------------------------------------------------------------------------|
SUB check_lifeTap
  /if (${Debug}) /echo |- check_LifeTap ==>
  /if (${Assisting} && ${Defined[lifeTaps2D]}) {
    /declare s int local
    /for s 1 to ${lifeTaps2D.Size[1]}
    /call check_Ready "lifeTaps2D" ${s}
    /if (${c_Ready} && ${lifeTaps2D[${s},${iIfs}]}) {
      /if (${check_Mana["lifeTaps2D",${s}]}) {
        /if (${Me.PctHPs} <= ${lifeTaps2D[${s},${iHealPct}]}) {
          /if (${check_Distance[${AssistTarget},${lifeTaps2D[${s},${iMyRange}]}]}) {
            /call e3_Cast ${AssistTarget} "lifeTaps2D" ${s}
          }
        }
      }
    }
    /if (!${ActionTaken}) /next s
  }
  /if (${Debug}) /echo <== check_LifeTap -|
/RETURN

|--------------------------------------------------------------------------------|
#EVENT SpellSet "<#1#> SpellSet #2#"
#EVENT SpellSet "[#1#] SpellSet #2#"
#EVENT SpellSet "#1# tells you, 'SpellSet #2#'"
#EVENT SpellSet "#1# tells the group, 'SpellSet #2#'"
#EVENT SpellSet "#1# tell your party, 'SpellSet #2#'"
SUB event_SpellSet(line, ChatSender, spellArray)
/if (${Debug} || ${Debug_Assists})  /echo |- event_SpellSet ==>
  /if (${line.Find[,]}) /varset line ${line.Left[${Math.Calc[${line.Find[,]}-1]}]} ${line.Right[-${line.Find[,]}]}
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${spellArray}]}) /return
  /varset spellArray ${c_eventArgData}
  /declare i int local
  /if (${Select[${Me.Class.ShortName},BST,CLR,DRU,ENC,MAG,PAL,NEC,RNG,SHD,SHM,WIZ]}) {
    /if (${Select[${spellArray},ON,TRUE,Nuke]}) {
      /varset spellArray Main
    } else /if (${Select[${spellArray},OFF,FALSE]}) {
      /varset use_Nukes FALSE
      /echo \ag Nuking Disabled
    }
    |nukes
    /if (!${Ini[${Character_Ini},Nukes,${spellArray}].Length}) {
      /if (${Debug} || ${Debug_Assists}) /echo \ay [${spellArray}] is not configured in your bot ini [Nukes], keeping existing spell set: [${spellsToUse}]
    } else {
      /call IniToArrayV "${Character_Ini},Nukes,${spellArray}#" MyNukes
      /if (${Defined[MyNukes]}) {
        /if (${MyNukes[1].Equal[OFF]}) {
          /varset use_Nukes FALSE
          /echo \ag Nuking Disabled, ${spellArray} set to OFF
        } else {
          /call BuildSpellArray "MyNukes" "Nukes2D"
          /varset spellsToUse ${spellArray}
          /varset use_Nukes TRUE
          /echo \ag Nukes set to [${spellsToUse}] - ${Nukes2D.Size[1]} spells
          /if (!${Following}) {
            /for i 1 to ${Nukes2D.Size[1]}
              /if (!${Bool[${Me.Gem[${Nukes2D[${i},${iCastName}]}]}]}) {
                /call memorize_spell "${Nukes2D[${i},${iCastName}]}" ${Nukes2D[${i},${iSpellGem}]}
              }
            /next i
          }
        }
      }
    }
    |dots on assist
    /if (${Ini[${Character_Ini},DoTs on Assist].Length}) {
      /if (!${Ini[${Character_Ini},Dots on Assist,${spellArray}].Length}) {
        /if (${Debug} || ${Debug_Assists}) /echo \ay [${spellArray}] is not configured in your bot ini [Dots on Assist], keeping existing spell set: [${spellsToUse}]
      } else {
        /call IniToArrayV "${Character_Ini},Dots on Assist,${spellArray}#" Assist_DoTs
        /if (${Defined[Assist_DoTs]}) {
          /if (${Assist_DoTs[1].Equal[OFF]}) {
            /varset use_AssistDots FALSE
            /echo \ag Assist Dotting Disabled, ${spellArray} set to OFF
          } else {
            /call BuildSpellArray "Assist_DoTs" "Assist_DoTs2D"
            /varset spellsToUse ${spellArray}
            /varset use_AssistDots TRUE
            /echo \ag Dots on Assist set to [${spellsToUse}] - ${Assist_DoTs2D.Size[1]} spells
            /if (!${Following}) {
              /for i 1 to ${Assist_DoTs2D.Size[1]}
                /if (!${Bool[${Me.Gem[${Assist_DoTs2D[${i},${iCastName}]}]}]}) {
                  /call memorize_spell "${Assist_DoTs2D[${i},${iCastName}]}" ${Assist_DoTs2D[${i},${iSpellGem}]}
                }
              /next i
            }
          }
        }
      }
    }
    |dots on command
    /if (${Ini[${Character_Ini},DoTs on Command].Length}) {
      /if (!${Ini[${Character_Ini},Dots on Command,${spellArray}].Length}) {
        /if (${Debug} || ${Debug_Assists}) /echo \ay [${spellArray}] is not configured in your bot ini [Dots on Command], keeping existing spell set: [${spellsToUse}]
      } else {
        /call IniToArrayV "${Character_Ini},Dots on Command,${spellArray}#" Command_DoTs
        /if (${Defined[Command_DoTs]}) {
          /if (${Command_DoTs[1].Equal[OFF]}) {
            /varset use_CommandDots FALSE
            /echo \ag Command Dotting Disabled, ${spellArray} set to OFF
          } else {
            /call BuildSpellArray "Command_DoTs" "Command_DoTs2D"
            /varset spellsToUse ${spellArray}
            /varset use_CommandDots TRUE
            /echo \ag Dots on Command set to [${spellsToUse}] - ${Command_DoTs2D.Size[1]} spells
            /if (!${Following}) {
              /for i 1 to ${Command_DoTs2D.Size[1]}
              /if (!${Bool[${Me.Gem[${Command_DoTs2D[${i},${iCastName}]}]}]}) {
                /call memorize_spell "${Command_DoTs2D[${i},${iCastName}]}" ${Command_DoTs2D[${i},${iSpellGem}]}
              }
              /next i
            }
          }
        }
      }
    }
  } else /if (${Me.Class.ShortName.Equal[ROG]}) {
    /declare s int local
    /declare cbpi int local
    /declare sbpi int local
    /call IniToArrayV "${Character_Ini},Buffs,Combat Buff#"	CombatBuffs
    /if (${Defined[CombatBuffs]}) {
      /for s 1 to ${CombatBuffs.Size}
        /if (${Select[${CombatBuffs[${s}]},${poisonFR},${poisonCR},${poisonPR}]}) {
          /varset cbpi ${s}
          /if (${Debug} || ${Debug_Assists}) /echo poison found in ${cbpi} ${CombatBuffs[${cbpi}]}
        }
      /next s
    }
    /call IniToArrayV "${Character_Ini},Buffs,Self Buff#" SelfBuffs
    /if (${Defined[SelfBuffs]}) {
      /for s 1 to ${SelfBuffs.Size}
        /if (${Select[${SelfBuffs[${s}]},${poisonFR},${poisonCR},${poisonPR}]}) {
          /varset sbpi ${s}
          /if (${Debug} || ${Debug_Assists}) /echo poison found in ${sbpi} ${SelfBuffs[${sbpi}]}
        }
      /next s
    }
    /if (${spellArray.Find[poison]}) {
      /if (${FindItemCount[=${poisonPR}]}) {
        /if (${cbpi}) /varset CombatBuffs[${cbpi}] ${poisonPR}/${Me.Name}
        /if (${sbpi}) /varset SelfBuffs[${sbpi}] ${poisonPR}
        /squelch ${Me.Buff[${FindItem[=${poisonFR}].Spell}].Remove}
        /squelch ${Me.Buff[${FindItem[=${poisonCR}].Spell}].Remove}
        /echo \ag Poison set to: ${poisonPR}
      } else {
        /echo \ay I do not have any ${poisonPR}, poison unchanged
      }
    } else /if (${spellArray.Find[fire]}) {
      /if (${FindItemCount[=${poisonFR}]}) {
        /if (${cbpi}) /varset CombatBuffs[${cbpi}] ${poisonFR}/${Me.Name}
        /if (${sbpi}) /varset SelfBuffs[${sbpi}] ${poisonFR}
        /squelch ${Me.Buff[${FindItem[=${poisonPR}].Spell}].Remove}
        /squelch ${Me.Buff[${FindItem[=${poisonCR}].Spell}].Remove}
        /echo \ag Poison set to: ${poisonFR}
      } else {
        /echo \ay I do not have any ${poisonFR}, poison unchanged
      }
    } else /if (${spellArray.Find[cold]}) {
      /if (${FindItemCount[=${poisonCR}]}) {
        /if (${cbpi}) /varset CombatBuffs[${cbpi}]  ${poisonCR}/${Me.Name}
        /if (${sbpi}) /varset SelfBuffs[${sbpi}]    ${poisonCR}
        /squelch ${Me.Buff[${FindItem[=${poisonPR}].Spell}].Remove}
        /squelch ${Me.Buff[${FindItem[=${poisonFR}].Spell}].Remove}
        /echo \ag Poison set to: ${poisonCR}
      } else {
        /echo \ay I do not have any ${poisonCR}, poison unchanged
      }
    } else {
      /squelch ${Me.Buff[${FindItem[=${poisonPR}].Spell}].Remove}
      /squelch ${Me.Buff[${FindItem[=${poisonFR}].Spell}].Remove}
      /squelch ${Me.Buff[${FindItem[=${poisonCR}].Spell}].Remove}
    }
    /if (${Defined[CombatBuffs]}) /call BuildSpellArray "CombatBuffs"	"CombatBuffs2D"
    /if (${Defined[SelfBuffs]})   /call BuildSpellArray "SelfBuffs"	  "SelfBuffs2D"
  }
/if (${Debug} || ${Debug_Assists})  /echo <== event_SpellSet -|
/RETURN

|--------------------------------------------------------------------------------|
#event CombatMode "<#1#> CombatMode #2#"
sub event_CombatMode(line, ChatSender, combatMode)
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${combatMode}]}) /return
  /varset combatMode ${c_eventArgData}
  /if (${combatMode.Equal[melee]}) {
    /varset AssistType Melee
    /delay 2
    /swap melee
    /delay 2
  }
  /if (${combatMode.Equal[ranged]}) {
    /varset AssistType Ranged
    /delay 2
    /swap ranged
    /delay 2
  }
  /if (${combatMode.Equal[off]}) {
    /varset AssistType off
  }
  /if (${Debug} || ${Debug_Assists})  /echo |-- Leaving Combat Mode ---|
/return

|--------------------------------------------------------------------------------|
Sub assist_Setup
/call iniToVarV "${advSettings_Ini},Debug,Debug Assists (On/Off)" Debug_Assists bool outer
/if (${Debug} || ${Debug_Assists}) /echo |- assist_Setup ==>

	/declare TogglePvP bool outer FALSE
	/declare Assisting bool outer FALSE
	/declare AllowControl bool outer FALSE
  /declare AssistTarget int outer 0
	/declare AssistStickDistance int outer
	/declare PauseTarget string outer
|	/declare AutoAggro bool outer TRUE
	/declare use_Nukes bool outer TRUE
	/declare use_TargetAE bool outer FALSE
	/declare use_PBAE bool outer FALSE
	/declare use_AssistDots bool outer TRUE
  /declare use_CommandDots bool outer TRUE

  /declare use_FULLBurns bool outer FALSE
  /declare use_LONGBurns bool outer FALSE
  /declare use_QUICKBurns bool outer FALSE
  /declare use_EPICBurns bool outer FALSE

  /declare resistCount string outer
	/declare isEnraged bool outer FALSE
	/declare revertRngToMelee bool outer FALSE
	/declare pauseRanged bool outer FALSE
  /declare aeTarget int outer
  /if (!${Defined[spellsToUse]})   /declare spellsToUse string global Main

| ------------------------------------- Import Macro_Ini Settings.
	/call iniToVarV "${genSettings_Ini},Assists,Max Engage Distance"     MaxAssistDistance int outer
	/call iniToVarV "${genSettings_Ini},Assists,AE Threat Range"         AE_Radius int outer
	/call iniToVarV "${genSettings_Ini},Assists,Acceptable Target Types" AcceptableTargetTypes string outer
	/call iniToVarV "${genSettings_Ini},Assists,Auto-Assist (On/Off)"    auto_AssistOn bool outer
  /call iniToVarV "${genSettings_Ini},Assists,Long Term Debuff Recast(s)" longTermRecastTime int outer
  /call iniToVarV "${genSettings_Ini},Assists,Short Term Debuff Recast(s)" shortTermRecastTime int outer

| ------------------------------------- Import Character_Ini Settings.
	/if (${Ini[${Character_Ini},Assist Settings,Assist Type (Melee/Ranged/Off)].Length})  /call iniToVarV "${Character_Ini},Assist Settings,Assist Type (Melee/Ranged/Off)" AssistType string outer
  /if (${Ini[${Character_Ini},Assist Settings,Taunt(On/Off)].Length})                   /call iniToVarV "${Character_Ini},Assist Settings,Taunt(On/Off)" doTaunt bool outer
  /if (${Ini[${Character_Ini},Assist Settings,SmartTaunt(On/Off)].Length})              /call iniToVarV "${Character_Ini},Assist Settings,SmartTaunt(On/Off)" smartTaunt bool outer
	/if (${Ini[${Character_Ini},Assist Settings,Melee Stick Point].Length})               /call iniToVarV "${Character_Ini},Assist Settings,Melee Stick Point" AssistStickPoint string outer
	/if (${Ini[${Character_Ini},Assist Settings,Melee Distance].Length})                  /call iniToVarV "${Character_Ini},Assist Settings,Melee Distance" AssistDistanceSetting string outer
	/if (${Ini[${Character_Ini},Assist Settings,Ranged Distance].Length})                 /call iniToVarV "${Character_Ini},Assist Settings,Ranged Distance" RangedStickDistance string outer
	/if (${Ini[${Character_Ini},Assist Settings,Auto-Assist Engage Percent].Length})      /call iniToVarV "${Character_Ini},Assist Settings,Auto-Assist Engage Percent" EngagePct int outer
	/if (!${Defined[EngagePct]}) {
		/if (${Debug} || ${Debug_Assists}) /echo ERROR: @[EngagePct] - EngagePct is invalid, defaulting to 98%
		/declare EngagePct int outer 98
	} else /if (${Defined[EngagePct]} && !${EngagePct}) {
		/if (${Debug} || ${Debug_Assists}) /echo ERROR: @[EngagePct] - EngagePct is invalid, defaulting to 98%
		/varset EngagePct 98
	}

  | Epic Definitions for EpicBurn -exclude defensive or buffs
  /declare EpicName string outer
  /if (${FindItemCount[=Prismatic Dragon Blade]})                /varset EpicName Prismatic Dragon Blade
  /if (${FindItemCount[=Blade of Vesagran]})                     /varset EpicName Blade of Vesagran
  /if (${FindItemCount[=Raging Taelosian Alloy Axe]})            /varset EpicName Raging Taelosian Alloy Axe
  /if (${FindItemCount[=Vengeful Taelosian Blood Axe]})          /varset EpicName Vengeful Taelosian Blood Axe
  /if (${FindItemCount[=Staff of Living Brambles]})              /varset EpicName Staff of Living Brambles
  /if (${FindItemCount[=Staff of Everliving Brambles]})          /varset EpicName Staff of Everliving Brambles
  /if (${FindItemCount[=Fistwraps of Celestial Discipline]})     /varset EpicName Fistwraps of Celestial Discipline
  /if (${FindItemCount[=Transcended Fistwraps of Immortality]})  /varset EpicName Transcended Fistwraps of Immortality
  /if (${FindItemCount[=Redemption]})                            /varset EpicName Redemption
  /if (${FindItemCount[=Nightbane, Sword of the Valiant]})       /varset EpicName Nightbane, Sword of the Valiant
  /if (${FindItemCount[=Heartwood Blade]})                       /varset EpicName Heartwood Blade
  /if (${FindItemCount[=Aurora, the Heartwood Blade]})           /varset EpicName Aurora, the Heartwood Blade
  /if (${FindItemCount[=Fatestealer]})                           /varset EpicName Fatestealer
  /if (${FindItemCount[=Nightshade, Blade of Entropy]})          /varset EpicName Nightshade, Blade of Entropy
  /if (${FindItemCount[=Innoruuk's Voice]})                      /varset EpicName Innoruuk's Voice
  /if (${FindItemCount[=Innoruuk's Dark Blessing]})              /varset EpicName Innoruuk's Dark Blessing
  /if (${FindItemCount[=Crafted Talisman of Fates]})             /varset EpicName Crafted Talisman of Fates
  /if (${FindItemCount[=Blessed Spiritstaff of the Heyokah]})    /varset EpicName Blessed Spiritstaff of the Heyokah
  /if (${FindItemCount[=Staff of Prismatic Power]})              /varset EpicName Staff of Prismatic Power
  /if (${FindItemCount[=Staff of Phenomenal Power]})             /varset EpicName Staff of Phenomenal Power
  /if (${FindItemCount[=Soulwhisper]})                           /varset EpicName Soulwhisper
  /if (${FindItemCount[=Deathwhisper]})                          /varset EpicName Deathwhisper
  /if (${EpicName.Length})     /call BuildArray EPICBurns "${EpicName}"

  /declare AnguishChest string outer
  /if (${FindItemCount[=Bladewhisper Chain Vest of Journeys]})                /varset AnguishChest Bladewhisper Chain Vest of Journeys
  /if (${FindItemCount[=Farseeker's Plate Chestguard of Harmony]})            /varset AnguishChest Farseeker's Plate Chestguard of Harmony
  /if (${FindItemCount[=Wrathbringer's Chain Chestguard of the Vindicator]})  /varset AnguishChest Wrathbringer's Chain Chestguard of the Vindicator
  /if (${FindItemCount[=Savagesoul Jerkin of the Wilds]})                     /varset AnguishChest Savagesoul Jerkin of the Wilds
  /if (${FindItemCount[=Glyphwielder's Tunic of the Summoner]})               /varset AnguishChest Glyphwielder's Tunic of the Summoner
  /if (${FindItemCount[=Whispering Tunic of Shadows]})                        /varset AnguishChest Whispering Tunic of Shadows
  /if (${AnguishChest.Length}) /call BuildArray EPICBurns "${AnguishChest}"

  |/echo ---------Assist calling spellarrays ----------
  /call assist_SpellArrays

  | Setup a spammable spell for spell procs such as deep sleep and languid bite
  /if (!${Defined[spammableSpell]}) {
    /call iniToVarV "${Character_Ini},Misc,Spammable Spell" spammableSpell string outer
    /if (${Bool[${spammableSpell}]}) {
      /echo I can now spam ${spammableSpell}, please use a /CheckFor to stop once the proc spell is on
    }
  }

  /if (${Debug} || ${Debug_Assists}) /echo <== assist_Setup -|
/return

|------------------------------------------------------------------------------------------------
|------------------------------------------------------------------------------------------------
SUB assist_SpellArrays
  /call IniToArrayV "${Character_Ini},Melee Abilities,Ability#"         MyAbilities
  /call IniToArrayV "${Character_Ini},Nukes,${spellsToUse}#"            MyNukes
  /call IniToArrayV "${Character_Ini},DoTs on Assist,${spellsToUse}#"   Assist_DoTs
  /call IniToArrayV "${Character_Ini},DoTs on Command,${spellsToUse}#"  Command_DoTs
  /call IniToArrayV "${Character_Ini},Debuffs,Debuff on Assist#"        Assist_Debuffs
  /call IniToArrayV "${Character_Ini},Debuffs,Debuff on Command#"       Command_Debuffs
  /call IniToArrayV "${Character_Ini},Debuffs,Debuff on All#"           All_Debuffs
  /call IniToArrayV "${Character_Ini},TargetAE,TargetAE#"               TargetAE_Spells
  /call IniToArrayV "${Character_Ini},PBAE,PBAE#"                       PBAE_Spells
  /call IniToArrayV "${Character_Ini},Burn,Quick Burn#"                 QUICKBurns
  /call IniToArrayV "${Character_Ini},Burn,Long Burn#"                  LONGBurns
  /call IniToArrayV "${Character_Ini},Burn,Full Burn#"                  FULLBurns
  /call IniToArrayV "${Character_Ini},Off Assist Spells,${spellsToUse}#"            OffAssistSpells

  /if (${Me.Class.ShortName.Equal[MAG]}) {
	/if (!${Defined[summonMoltenOrb]}) /declare summonMoltenOrb bool outer FALSE
  /if (${Bool[${Me.AltAbility[Servant of Ro]}]})        /call BuildArray "swarmPets" "Servant of Ro"
  /if (${Bool[${Me.AltAbility[Host of the Elements]}]}) /call BuildArray "swarmPets" "Host of the Elements"

  /if (${Defined[MyNukes]}) {
    /declare i int local
    /for i 1 to ${MyNukes.Size}
      /if (${MyNukes[${i}].Find[Lava Orb]}) /varset summonMoltenOrb TRUE
    /next i
  }

  } else /if (${Me.Class.ShortName.Equal[NEC]}) {
    /if (${Bool[${Me.AltAbility[Swarm of Decay]}]}) /call BuildArray "swarmPets" "Swarm of Decay"
	  /if (${Bool[${Me.AltAbility[Rise of Bones]}]})  /call BuildArray "swarmPets" "Rise of Bones"
    /if (${Bool[${FindItem[Graverobber's Icon]}]}) /call BuildArray "swarmPets" "Graverobber's Icon"
    /if (${Bool[${FindItem[Soulwhisper]}]}) /call BuildArray "swarmPets" "Soulwhisper"
    /if (${Bool[${FindItem[Deathwhisper]}]}) /call BuildArray "swarmPets" "Deathwhisper"
    |Ewiclip: Removing Wake The Dead -- It is known to cause zone crashing
    |/if (${Bool[${Me.AltAbility[Wake the Dead]}]})  /call BuildArray "swarmPets" "Wake the Dead"
  } else /if (${Me.Class.ShortName.Equal[SHM]}) {
    /if (${Bool[${Me.AltAbility[Spirit Call]}]})    /call BuildArray "swarmPets" "Spirit Call"
    /if (${Bool[${FindItem[Shattered Gnoll Slayer]}]})    /call BuildArray "swarmPets" "Shattered Gnoll Slayer"
  } else /if (${Me.Class.ShortName.Equal[WIZ]}) {
    /if (${Bool[${Me.AltAbility[Call of Xuzl]}]})   /call BuildArray "swarmPets" "Call of Xuzl"
  } else /if (${Me.Class.ShortName.Equal[BRD]}) {
    /if (${Bool[${Me.AltAbility[Song of Stone]}]})   /call BuildArray "swarmPets" "Song of Stone"
  } else /if (${Me.Class.ShortName.Equal[CLR]}) {
    /if (${Bool[${Me.AltAbility[Celestial Hammer]}]}) /call BuildArray "swarmPets" "Celestial Hammer"
    /if (${Bool[${FindItem[Graverobber's Icon]}]}) /call BuildArray "swarmPets" "Graverobber's Icon"
  } else /if (${Me.Class.ShortName.Equal[ENC]}) {
    /if (${Bool[${Me.AltAbility[Doppelganger]}]}) {
      |/call BuildArray "swarmPets" "Doppelganger"
    }
    /if (${Bool[${Me.AltAbility[Phantasmal Opponent]}]}) {
      /call BuildArray "swarmPets" "Phantasmal Opponent"
    }
  } else /if (${Me.Class.ShortName.Equal[PAL]}) {
    |/if (${Bool[${Me.AltAbility[Projection of Piety]}]}) /call BuildArray "swarmPets" "Projection of Piety"
  } else /if (${Me.Class.ShortName.Equal[DRU]}) {
    /if (${Bool[${Me.AltAbility[Spirits of Nature]}]}) /call BuildArray "swarmPets" "Spirits of Nature"
    /if (${Bool[${Me.AltAbility[Nature's Guardian]}]}) /call BuildArray "swarmPets" "Nature's Guardian"
    /if (${Bool[${FindItem[Shattered Gnoll Slayer]}]}) /call BuildArray "swarmPets" "Shattered Gnoll Slayer"
  }

  /if (${Defined[MyAbilities]})		    /call BuildSpellArray "MyAbilities"	      "Abilities2D"
  /if (${Defined[MyNukes]})		        /call BuildSpellArray "MyNukes"	          "Nukes2D"
  /if (${Defined[PBAE_Spells]})		    /call BuildSpellArray "PBAE_Spells"	      "PBAE_Spells2D"
  /if (${Defined[TargetAE_Spells]})		/call BuildSpellArray "TargetAE_Spells" 	"TargetAE_Spells2D"
  /if (${Defined[Assist_Debuffs]})		/call BuildSpellArray "Assist_Debuffs" 		"Assist_Debuffs2D"
  /if (${Defined[Command_Debuffs]})		/call BuildSpellArray "Command_Debuffs" 	"Command_Debuffs2D"
  /if (${Defined[All_Debuffs]})		    /call BuildSpellArray "All_Debuffs" 	    "All_Debuffs2D"
  /if (${Defined[Assist_DoTs]})	      /call BuildSpellArray "Assist_DoTs" 	    "Assist_DoTs2D"
  /if (${Defined[Command_DoTs]})	    /call BuildSpellArray "Command_DoTs" 	    "Command_DoTs2D"
  /if (${Defined[EPICBurns]})         /call BuildSpellArray "EPICBurns" 	      "EPICBurns2D"
  /if (${Defined[QUICKBurns]})	      /call BuildSpellArray "QUICKBurns" 	      "QUICKBurns2D"
  /if (${Defined[LONGBurns]})	        /call BuildSpellArray "LONGBurns" 	      "LONGBurns2D"
  /if (${Defined[FULLBurns]})	        /call BuildSpellArray "FULLBurns" 	      "FULLBurns2D"
  /if (${Defined[swarmPets]})	        /call BuildSpellArray "swarmPets" 	      "swarmPets2D"
  /if (${Defined[OffAssistSpells]})	  /call BuildSpellArray "OffAssistSpells"   "OffAssist_Spells2D"
/RETURN

|--------------------------------------------------------------------------------|
SUB assist_MacroSettings
/if (${Debug}) /echo |- assist_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},Debug,Debug Assists (On/Off)" Off
	/call WriteToIni "${genSettings_Ini},Assists,Auto-Assist (On/Off)" Off
	/call WriteToIni "${genSettings_Ini},Assists,Max Engage Distance" 250
	/call WriteToIni "${genSettings_Ini},Assists,AE Threat Range" 100
	/call WriteToIni "${genSettings_Ini},Assists,Acceptable Target Types" NPC,Pet
  /call WriteToIni "${genSettings_Ini},Assists,Long Term Debuff Recast(s)" 30
  /call WriteToIni "${genSettings_Ini},Assists,Short Term Debuff Recast(s)" 5
/if (${Debug}) /echo <== assist_MacroSettings -|
/RETURN
|--------------------------------------------------------------------------------|
SUB assist_CharacterSettings
/if (${Debug}) /echo |- assist_CharacterSettings ==>
	| Add Assist_Stick entries.
	/call WriteToIni "${Character_Ini},Assist Settings,Assist Type (Melee/Ranged/Off)" ${If[${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR]},Melee,Off]}
  | Default Tanks to Front
  /if (${Select[${Me.Class.ShortName},PAL,SHD,WAR]}) {
    /call WriteToIni "${Character_Ini},Assist Settings,Melee Stick Point" Front
    /call WriteToIni "${Character_Ini},Assist Settings,Taunt(On/Off)" Off
    /call WriteToIni "${Character_Ini},Assist Settings,SmartTaunt(On/Off)" On
  } else {
    /call WriteToIni "${Character_Ini},Assist Settings,Melee Stick Point" Behind
  }
	/call WriteToIni "${Character_Ini},Assist Settings,Melee Distance" MaxMelee
	/call WriteToIni "${Character_Ini},Assist Settings,Ranged Distance" 100
  /call WriteToIni "${Character_Ini},Assist Settings,Auto-Assist Engage Percent" 98
	| If I'm a melee class, add Melee Abilities entries.
	/if (${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR,CLR]}) {
		/if (!${Ini[${Character_Ini},Melee Abilities].Length}) /call WriteToIni "${Character_Ini},Melee Abilities,Ability"
	}
	| If I'm a casting class, add Nuke entries.
	/if (${Select[${Me.Class.ShortName},BST,CLR,DRU,ENC,MAG,PAL,NEC,RNG,SHD,SHM,WIZ]}) {
		/if (!${Ini[${Character_Ini},Nukes].Length}) {
      /call WriteToIni "${Character_Ini},Nukes,Main"
      /if (${Select[${Me.Class.ShortName},DRU,RNG,MAG,WIZ]}) {
        /call WriteToIni "${Character_Ini},Nukes,BigFire"
        /call WriteToIni "${Character_Ini},Nukes,FastFire"
        /call WriteToIni "${Character_Ini},Nukes,LureFire"
      }
      /if (${Select[${Me.Class.ShortName},RNG,WIZ,SHM]}) {
        /call WriteToIni "${Character_Ini},Nukes,BigCold"
        /call WriteToIni "${Character_Ini},Nukes,FastCold"
        /call WriteToIni "${Character_Ini},Nukes,LureCold"
      }
      /if (${Select[${Me.Class.ShortName},MAG,WIZ]}) {
        /call WriteToIni "${Character_Ini},Nukes,BigMagic"
        /call WriteToIni "${Character_Ini},Nukes,FastMagic"
        /call WriteToIni "${Character_Ini},Nukes,LureMagic"
      }
    }
	}

	| If I'm a casting class, add DoT entries.
  /if (${Ini[${Character_Ini},DoTs].Length}) {
    /call WriteToIni "${Character_Ini},DoTs,The [Dots] section is deprecated, move dots to [DoTs on Command] and [DoTs on Assist]"
    /bc [+r+] The [Dots] section is deprecated, move dots to [DoTs on Command] and [DoTs on Assist]
  }

	/if (${Select[${Me.Class.ShortName},BST,DRU,ENC,NEC,RNG,SHD,SHM]}) {
    /if (!${Ini[${Character_Ini},DoTs on Assist].Length}) {
      /call WriteToIni "${Character_Ini},DoTs on Assist,Main"
      /if (${Select[${Me.Class.ShortName},DRU,ENC,NEC,SHD,SHM,RNG]}) /call WriteToIni "${Character_Ini},DoTs on Assist,Magic"
      /if (${Select[${Me.Class.ShortName},NEC,SHD,SHM,BST]}) /call WriteToIni "${Character_Ini},DoTs on Assist,Poison"
      /if (${Select[${Me.Class.ShortName},NEC,SHD,SHM,BST]}) /call WriteToIni "${Character_Ini},DoTs on Assist,Disease"
      /if (${Select[${Me.Class.ShortName},DRU,NEC]}) /call WriteToIni "${Character_Ini},DoTs on Assist,Fire"
    }
    /if (!${Ini[${Character_Ini},DoTs on Command].Length}) {
      /call WriteToIni "${Character_Ini},DoTs on Command,Main"
      /if (${Select[${Me.Class.ShortName},DRU,ENC,NEC,SHD,SHM,RNG]}) /call WriteToIni "${Character_Ini},DoTs on Command,Magic"
      /if (${Select[${Me.Class.ShortName},NEC,SHD,SHM,BST]}) /call WriteToIni "${Character_Ini},DoTs on Command,Poison"
      /if (${Select[${Me.Class.ShortName},NEC,SHD,SHM,BST]}) /call WriteToIni "${Character_Ini},DoTs on Command,Disease"
      /if (${Select[${Me.Class.ShortName},DRU,NEC]}) /call WriteToIni "${Character_Ini},DoTs on Command,Fire"
    }
	}
	| If I'm a casting class, add Debuff entries.
	/if (${Select[${Me.Class.ShortName},BST,DRU,ENC,MAG,NEC,SHM,RNG]}) {
		/if (!${Ini[${Character_Ini},Debuffs].Length}) {
			/call WriteToIni "${Character_Ini},Debuffs,Debuff on Assist"
			/call WriteToIni "${Character_Ini},Debuffs,Debuff on Command"
		}
	}
	| If I'm a casting class, add AE entries.
	/if (${Select[${Me.Class.ShortName},CLR,ENC,DRU,MAG,SHM,WIZ,RNG]}) {
		/if (!${Ini[${Character_Ini},TargetAE].Length}) /call WriteToIni "${Character_Ini},TargetAE,TargetAE"
    /if (!${Ini[${Character_Ini},PBAE].Length}) /call WriteToIni "${Character_Ini},PBAE,PBAE"
	}
	| Burn entries.
	/if (!${Ini[${Character_Ini},Burn].Length}) {
		/call WriteToIni "${Character_Ini},Burn,Quick Burn"
    /call WriteToIni "${Character_Ini},Burn,Long Burn"
		/call WriteToIni "${Character_Ini},Burn,Full Burn"
	}
/if (${Debug}) /echo <== assist_CharacterSettings -|
/RETURN

|--------------------------------------------------------------------------------|
Sub assist_Aliases
/if (${Debug}) /echo |- assist_Aliases ==>
	/noparse /squelch /alias /AssistOn /bc Assist on ${Target.ID}
	/noparse /squelch /alias /AssistMe /bc Assist on ${Target.ID}
	/squelch /alias /BackOff /bc Back off
	/noparse /squelch /alias /debuff /bc Debuffs on ${Target.ID}
	/noparse /squelch /alias /debuffson /bc Debuffs on ${Target.ID}
	/squelch /alias /debuffsoff /bc End Debuffs
	/noparse /squelch /alias /dot /bc DoTs on ${Target.ID}
	/noparse /squelch /alias /dotson /bc DoTs on ${Target.ID}
	/squelch /alias /dotsoff /bc End DoTs
	/squelch /alias /targetaeon /bc targetae on
	/squelch /alias /targetaeoff /bc targetae off
	/squelch /alias /pbaeon /bc pbae on
  /squelch /alias /pbaeoff /bc pbae off
  /squelch /alias /aeon /bc allae on
  /squelch /alias /aeoff /bc allae off
  /squelch /alias /pvpOff /bc //varset TogglePvP FALSE
  /squelch /alias /pvpOn /bc //varset TogglePvP TRUE
  /noparse /squelch /alias /SwarmPets /bc Swarm pets on ${Target.ID}
	/squelch /alias /EpicBurns /bc Epic burn
	/squelch /alias /QuickBurns /bc Quick burn
  /squelch /alias /LongBurns /bc Long burn
	/squelch /alias /FullBurns /bc Full burn
	/squelch /alias /taunton /varset doTaunt 1
  /squelch /alias /tauntoff /varset doTaunt 0
	/squelch /alias /AssistType /bc //varset Assist Type
	/squelch /alias /ns /bc SpellSet
  /squelch /alias /ss /bc SpellSet
	/squelch /alias /combatmode /bc combatmode
	/squelch /alias /cm /bc combatmode

  /squelch /alias /Lesson /bc VetAA Lesson of the Devoted
  /squelch /alias /Infusion /bc VetAA Infusion of the Faithful
  /squelch /alias /Staunch /bc VetAA Staunch Recovery
  /squelch /alias /Servant /bc VetAA Steadfast Servant
  /squelch /alias /Intensity /bc VetAA Intensity of the Resolute
  /squelch /alias /Armor /bc VetAA Armor of Experience
  /squelch /alias /Expedient /bc VetAA Expedient Recovery
  |/squelch /alias /Throne /bc VetAA Throne of Heroes
  /squelch /alias /Jester /bc VetAA Chaotic Jester

/if (${Debug}) /echo <== assist_Aliases -|
/return

|--------------------------------------------------------------------------------|
Sub assist_Background_Events
 
  /if (${auto_AssistOn})    /call check_AutoAssist
  /if (${tankKillEngaged})  /call tankKill
  /doevents Assist
  /doevents BackOff
  /if (${Assisting}) {
    /call check_AssistStatus
    /varset combatTimer ${combatDelay}
  } else {
    /if (${Defined[timerArray]}) /call check_macroTimers
  }
  /doevents Burns
  /doevents AEs
  /doevents EnrageOn
  /doevents EnrageOff
  /doevents DebuffsOn
  /doevents DebuffsAllOn
  /doevents DebuffsOff
  /doevents DoTsOn
  /doevents DoTsOff
  /doevents SwarmPets
  /doevents SpellSet
  /doevents SetTank
  /doevents CombatMode
  /doevents debuffsOnAll
/return

#EVENT debuffsOnAll "<|${Me.Name}|> Debuff On All"
SUB event_debuffsOnAll(line)
  /declare i int local
  /for i 1 to 13
    /if (${Me.XTarget[${i}].TargetType.Equal[Auto Hater]} && ${Me.XTarget[${i}].LineOfSight} && ${Me.XTarget[${i}].Aggressive} && ${Me.XTarget[${i}].ID}) {
      /bc Debuff all on ${Me.XTarget[${i}].ID}
    }
  /next i
/RETURN

|Ewiclip MOD: Function for returning if a spell gem is empty
SUB GetEmptySpellGem
  /declare i int local 
  /for i 1 to ${Math.Calc[8+${Me.AltAbility[Mnemonic Retention].Rank}]}
    /if (!${Bool[${Me.Gem[${i}]}]}) {
      /RETURN ${i}
    }
  /next i
/RETURN 0

|----------------------------------------------------------------------|
|- Ewiclip MOD: Function for returning duration of my spell on the NPC
|- Return Seconds remaining for a buff on your target
|----------------------------------------------------------------------|
Sub TimeLeftOnMySpell(SpellName)
  |${Target.BuffDuration[${${ArrayName}[${i},${iCastName}]} ]} -- Divide by 1000 to get seconds remaining on DoTName
  |${Target.Buff[${${ArrayName}[${i},${iCastName}]} ].Caster}  -- Who casted this DoT?
  |${Target.Buff[DoTName].ID) -- SpellID 
  |${FindItem[DoTName].Spell.ID} -- Returns ID of spell on item correctly
  
  /declare SpellID string local 0
  /declare ItemName string local
  /declare BuffDuration int local 0

  /if (${Bool[${FindItem[=${SpellName}]}]}) {
    /varset SpellID ${FindItem[${SpellName}].Spell.ID}
    /varset ItemName SpellName
    /varset SpellName ${FindItem[${SpellName}].Spell}
  } else {
    /varset SpellID ${Spell[${SpellName}].ID}
  }
  
  /declare iBuff int local 1
  /for iBuff 1 to 100
    /if ((${SpellName.Equal[${Target.Buff[${iBuff}]}]}) && ${SpellID.Equal[${Target.Buff[${iBuff}].ID}]} && ${Target.Buff[${iBuff}].Caster.Equal[${Me}]}) {
      /varset BuffDuration ${Math.Calc[${Target.BuffDuration[${iBuff}]} / 1000]}
      /break
    }
  /next iBuff
/RETURN ${BuffDuration}