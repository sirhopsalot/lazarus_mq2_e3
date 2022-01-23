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
| Casts the spell at the front of the queue
SUB castNowSpells
    |a check to make double sure we notify the rest of the program we are not casting yet.
    /varset g_CastNowSpellCasting False
    /declare spellArgs string local 
    /declare spellName string local
    /declare spellTargetID int local
    
    /while (${nowSpellArrayQueueCount}>0) {
        |get the value out of the queue
        /call NowCast_CircularQueue_Pop
        /varset spellArgs ${Macro.Return}
        
        /varset spellName ${spellArgs.Arg[1,,]}
        /varset spellTargetID ${spellArgs.Arg[2,,]}

        /if (${DebugNowCast}) /echo spellName:${spellName}
        /if (${spellTargetID}==0) {
            /echo nowCast targetID is 0, Target was not supplied correctly
            /return
        }
        /if (${DebugNowCast}) /echo nowCast targetID: ${spellTargetID} 	
        | Check if SpellReady/AltAbilityReady

        /if (${Bool[${Me.Book[${spellName}]}]}) {
            |if this is a spell, delay up to 0.3 sec for the spell gems to be ready
            /delay 3 ${Me.SpellReady[${spellName}]}
        }
       
        /if (${Me.SpellReady[${spellName}]} || ${Me.AltAbilityReady[${spellName}]} || ${Me.ItemReady[${spellName}]}) {
            /call castNowSpell "${spellName}" ${spellTargetID}
            /if (!(${ActionTaken} && ${castReturn.Equal[CAST_SUCCESS]})) {
                /bc <NowCast>: Failed to cast ${spellName} due to ${castReturn}
            } else /if (${nowSpellArrayQueueCount}>0 ) {
                |give times global cooldown if this is not an item
                /if (!${Bool[${FindItemCount[=${spellName}]}]}) {
		           /delay 15
            	}	
                
            }
        } else {
            /bc <NowCast>: Failed to cast ${spellName}, Spell/Ability/Item not ready or not found
        } 
    }
/RETURN
| Calls e3_Cast for the given spell at the given target
SUB castNowSpell(spellName, targetID)

	/declare whatToCast string local
	/if (${Bool[${Me.Book[${spellName}]}]} || ${Bool[${Me.AltAbility[${spellName}]}]}) {
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
#event nowCast "[#1#(msg)]#2# nowCast #3#"
SUB event_nowCast(line, ChatSender, Caster, spellName)
    /if (${Bool[${Caster}]} &&!${Me.Name.Equal[${Caster}]}) /return

    /declare targetID int local ${Me.ID}
    /declare spellIndexStart int local 0
    /declare targetIDIndex int local 0

    /if (${Bool[${spellName.Length}]}) {
        /varset targetIDIndex ${spellName.Find[targetid]}
        /if (${targetIDIndex}>0) {
            /varset targetID ${spellName.Token[2,=]}
            /varset spellName ${spellName.Left[${Math.Calc[${targetIDIndex}-2]}]}       
        }
    }
    /if (${nowSpellArrayQueueCount}==${nowSpellArrayQueueMax}) {
        /echo CastNow Queue is maxed, dropping ${spellName},${targetID}
        /return
    }
    /call NowCast_CircularQueue_Insert "${spellName},${targetID}"
	|c_SubToRun is used internally by e3_Cast, we are flaggint that if something was casting, we would like to interrupt 
    |once we leave this  e3_Casting should check the flag and then call an interrupt
    /if (!${g_CastNowSpellCasting}) /varset c_SubToRun FALSE
/RETURN

