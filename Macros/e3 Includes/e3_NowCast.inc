|--------------------------------------------------------------------------------|
| Now Cast
|
| Author: Rekka
| This is for when you need something cast immediately. Unless its a Tank heal,
| Important heal, or XTarget Heal in process. This will stop whatever is being cast.
| by issuing an interrupt, which will kick out to the main event loop and then
| the very next method called will be castNowSpells. 
|--------------------------------------------------------------------------------|

|--------------------------------------------------------------------------------|
| Declare variables used in the macro
|--------------------------------------------------------------------------------|
Sub NowCastDefinedValues
    |Create a circular queue with insert and pop methods
    /declare nowSpellArrayQueue[10] string outer
    /declare nowSpellArrayQueueIndexFront int outer 0
    /declare nowSpellArrayQueueIndexBack int outer -1
    /declare nowSpellArrayQueueMax int outer 10
    /declare nowSpellArrayQueueCount int outer 0
 	/declare DebugNowCast bool outer FALSE
    |global variable to determine if this is a now cast spell while casting
    /declare g_CastNowSpellCasting bool outer FALSE
/RETURN
Sub NowCast_CircularQueue_Insert(InsertValue)

    /if (${nowSpellArrayQueueCount}==${nowSpellArrayQueueMax}) {
        /echo NowCast Queue Overflow, throwing away ${InsertValue}
        /return
    }
    /varset nowSpellArrayQueueIndexBack ${Math.Calc[(${nowSpellArrayQueueIndexBack} + 1) % ${nowSpellArrayQueueMax}]}
    |Array indexes starting at 1 is stupid and the person should be flogged with a wet fish
    /declare arrayIndex int local ${Math.Calc[${nowSpellArrayQueueIndexBack}+1]}
    |/echo Addding with array index of ${arrayIndex} with value ${InsertValue}
    /varset nowSpellArrayQueue[${arrayIndex}] ${InsertValue}
    /varcalc nowSpellArrayQueueCount ${nowSpellArrayQueueCount}+1

/return
Sub NowCast_CircularQueue_Pop
    /if (${nowSpellArrayQueueCount}==0) {
        /echo NowCast Queue Empty Already
        /return
    }
    /declare returnValue string local ${nowSpellArrayQueue[${Math.Calc[${nowSpellArrayQueueIndexFront}+1]}]}
    /varset nowSpellArrayQueueIndexFront ${Math.Calc[(${nowSpellArrayQueueIndexFront} + 1) % ${nowSpellArrayQueueMax}]}
    /varcalc nowSpellArrayQueueCount ${nowSpellArrayQueueCount}-1

/return ${returnValue}


Sub NowCast_CircularQueue_Empty
    /varset nowSpellArrayQueueCount 0
    /varset nowSpellArrayQueueIndexFront 0
    /varset nowSpellArrayQueueIndexBack -1

/return

Sub NowCast_CircularQueue_Peek
    /if (${nowSpellArrayQueueCount}==0) {
        /echo NowCast Queue Empty Already
        /return
    }
    /declare returnValue string local ${nowSpellArrayQueue[${Math.Calc[${nowSpellArrayQueueIndexFront}+1]}]}
    
/return ${returnValue}

|0 based index
Sub NowCast_CircularQueue_PeekIndex(indexValue)
    /if (${nowSpellArrayQueueCount}==0) {
        /echo NowCast Queue Empty Already
        /return
    }
   
    /declare arrayIndex int local  ${Math.Calc[(${nowSpellArrayQueueIndexFront} + ${indexValue} ) % ${nowSpellArrayQueueMax}]}
    /varCalc arrayIndex ${arrayIndex}+1
    /declare returnValue string local ${nowSpellArrayQueue[${arrayIndex}]}
    |/echo PeekIndex Front:${nowSpellArrayQueueIndexFront} back:${nowSpellArrayQueueIndexBack}  of input index: ${indexValue}, array index:${arrayIndex} return value: ${returnValue}

/return ${returnValue}

Sub NowCast_CircularQueue_PrintQueue
  /if (${nowSpellArrayQueueCount}==0) {
    /echo NowCast Queue Empty Already
    /return
  }
  /declare counter int local 0
  /declare itemValue string local

  |loop through the current queue,
  /while (${counter}< ${nowSpellArrayQueueCount}) {
    /call NowCast_CircularQueue_PeekIndex ${counter}
    /varset itemValue ${Macro.Return}
    /echo ${counter}) ${itemValue}
    /varcalc counter ${counter}+1
  }
    
/return




| Casts the spell at the front of the queue
SUB castNowSpells
    |a check to make double sure we notify the rest of the program we are not casting yet.
    /varset g_CastNowSpellCasting False
    /declare spellArgs string local 
    /declare spellName string local
    /declare spellTargetID int local
    /declare GCDOver bool local false

    /while (${nowSpellArrayQueueCount}>0) {
        |get the value out of the queue
        /call NowCast_CircularQueue_Pop
        /varset spellArgs ${Macro.Return}
        
         |/echo NowCast: pulling from queue:${spellArgs}

        /varset spellName ${spellArgs.Arg[1,,]}
        /varset spellTargetID ${spellArgs.Arg[2,,]}

        /if (${DebugNowCast}) /echo spellName:${spellName}
        /if (${spellTargetID}==0 || ${SpawnCount[id ${spellTargetID}]}==0) {
            /bc <nowCast> targetID is invalid: ${spellTargetID}
            /return
        }
        /if (${DebugNowCast}) /echo nowCast targetID: ${spellTargetID} 	
        | Check if SpellReady/AltAbilityReady

        /if (${Bool[${Me.Book[${spellName}]}]} && !${Me.Class.ShortName.Equal[BRD]}) {
           
            /varset GCDOver false
            /while (!${GCDOver}) {
                |/echo [${Time}] Now_cast: checking to see if GCD is over
                /call check_GCD 
                /varset GCDOver ${Macro.Return}
                /delay 1
            }
        } else /if (${Me.Class.ShortName.Equal[BRD]}) {
               
                /while (${Window[CastingWindow].Open}) {
                    /delay 1
                }
                /delay 1
                /stopcast
                /varset GCDOver false
                /while (!${GCDOver}) {
                    /call check_GCD 
                    /varset GCDOver ${Macro.Return}
                    /delay 1
                }
        }   
        /if (${Me.SpellReady[${spellName}]} || ${Me.AltAbilityReady[${spellName}]} || ${Me.ItemReady[=${spellName}]}|| ${Me.CombatAbilityReady[${spellName}]}) {
            /varset castReturn
             /bc <NowCast>: Casting ${spellName}  on ${spellTargetID}  
            /call castNowSpell "${spellName}" ${spellTargetID}
            /if (!(${ActionTaken} && ${castReturn.Equal[CAST_SUCCESS]})) {
                /bc [${Time}] <NowCast>: Failed to cast ${spellName} due to ${castReturn}
            } else /if (${nowSpellArrayQueueCount}>0 ) {
                |give times global cooldown if this is not an item
                /if (!${Bool[${FindItemCount[=${spellName}]}]}) {
		           /delay 15
            	}	
                
            }
        } else {
            /bc [${Time}] <NowCast>: Failed to cast ${spellName}, Spell/Ability/Item not ready or not found
        } 
    }
/RETURN
| Calls e3_Cast for the given spell at the given target
SUB castNowSpell(spellName, targetID)

	/declare whatToCast string local
	/if (${Bool[${Me.Book[${spellName}]}]} || ${Bool[${Me.AltAbility[${spellName}]}]} || ${Me.CombatAbilityReady[${spellName}]}) {
		/varset whatToCast ${spellName}
	} else /if (${Bool[${FindItemCount[=${spellName}]}]}) {
		/varset whatToCast ${FindItem[=${spellName}]}
	} 	
	/if (${Bool[${whatToCast}]}) {
        /varset g_CastNowSpellCasting TRUE
        /call castSimpleSpell "${whatToCast}" ${targetID}
        /varset g_CastNowSpellCasting False
       
	}
/RETURN
| Add a spell to the queue with no targetid
#event nowCast "<#1#> #2# nowCast #3#"
#event nowCast "<#1#> nowCast #3#"
#event nowCast "[#1#(msg)]#2# nowCast #3#"
SUB event_nowCast(line, ChatSender, Caster, spellName)
    |/echo <NowCast> called with Caster: ${Caster}
    /if (${Bool[${Caster}]} &&!${Me.Name.Equal[${Caster}]}) /return
     
    /if (!(${NetBots[${ChatSender}].ID} || ${Bool[${Raid.Member[${ChatSender}]}]})) {
        |Not one of my toons or part of my raid, ignore
        /bc <NowCast> ${ChatSender} outside of your group/raid tried to issue a nowcast for spell:${spellName}
        /return
    }

    /declare nowTargetID int local ${Me.ID}
    /declare spellIndexStart int local 0
    /declare nowTargetIDIndex int local 0

    /if (${Bool[${spellName.Length}]}) {
        /varset nowTargetIDIndex ${spellName.Find[targetid]}
        /if (${nowTargetIDIndex}>0) {
            /varset nowTargetID ${spellName.Token[2,=]}
            /varset spellName ${spellName.Left[${Math.Calc[${nowTargetIDIndex}-2]}]}       
        }
    }
    /if (${nowSpellArrayQueueCount}==${nowSpellArrayQueueMax}) {
        /echo CastNow Queue is maxed, dropping ${spellName},${nowTargetID}
        /return
    }
    |/echo NowCast: Inserting into queue:  "${spellName},${nowTargetID}"
    /call NowCast_CircularQueue_Insert "${spellName},${nowTargetID}"
	|c_SubToRun is used internally by e3_Cast, we are flaggint that if something was casting, we would like to interrupt 
    |once we leave this  e3_Casting should check the flag and then call an interrupt
    /if (!${g_CastNowSpellCasting}) /varset c_SubToRun FALSE
/RETURN

