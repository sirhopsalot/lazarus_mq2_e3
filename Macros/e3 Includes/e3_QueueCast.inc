|--------------------------------------------------------------------------------|
| Queue Cast
|
| Author: Sirhopsalot
| Contributors: Forsy, Warikan
|
| Queues a spell to be cast and retries with backoff times until it succeeds (or tries 4 times and a total of 15 seconds).
| This works into the e3 casting patterns so if the toon is busy casting right now it will
| cast at the next opportunity.
| 
| Usage (from any toon):
| 	/bc ToonName queueCast SpellName targetid=TargetID
| Usage to cast a spell with no target:
|   /bc Flora queueCast Circle of Knowledge
| Usage to cast a spell on a specific target:
|   /bc Glorious queueCast Holy Elixir targetid=${Me.ID}
|	/bc Oblivion queueCast Mind Flay targetid=${Target.ID}
|
| Installation:
| 1) Put e3_QueueCast.inc in the e3 Includes folder
| 2) In e3_Setup.inc, add: #include e3 Includes\e3_QueueCast.inc
| 3) In Advanced Settings.ini, add: Include#20=queueCast under [Included Setups]
|--------------------------------------------------------------------------------|

|--------------------------------------------------------------------------------|
| Declare variables used in the macro
|--------------------------------------------------------------------------------|
Sub queueCast_Setup
	/declare mySpellQueue string outer
	/declare queuedSpellTimer timer outer 0s
	/declare DebugQueueCast bool outer FALSE
/RETURN

|--------------------------------------------------------------------------------|
| Call background events and other subs required for checking the macro
|--------------------------------------------------------------------------------|
Sub queueCast_Background_Events
	/doevents queueCast
	/doevents queueCastNoTarget
	/call castQueuedSpells
/return

SUB queueCast_MacroSettings
/RETURN

SUB queueCast_CharacterSettings
/RETURN

SUB queueCast_Aliases
/RETURN

| Casts the spell at the front of the queue
SUB castQueuedSpells	
	/if (${Defined[mySpellQueue]} && ${mySpellQueue.Length} > 1 && !${Bool[${queuedSpellTimer}]}) {
		/call PeekStrQueue mySpellQueue		
		/if (${DebugQueueCast}) /echo Found spell '${Macro.Return}' to cast in queue
		/declare spellArgs string local ${Macro.Return}
		/declare spellName string local ${spellArgs.Arg[1,,]}
		/declare spellTargetID int local ${spellArgs.Arg[2,,]}	
		| Check if SpellReady/AltAbilityReady
		/if (${Me.SpellReady[${spellName}]} || ${Me.AltAbilityReady[${spellName}]} || ${Me.ItemReady[${spellName}]}) {
			/if (${DebugQueueCast}) /echo /call castQueuedSpell "${spellName}" ${spellTargetID}
			/call castQueuedSpell "${spellName}" ${spellTargetID}
			/if (${ActionTaken} && ${castReturn.Equal[CAST_SUCCESS]}) {
				/call PopStrQueue mySpellQueue
				/varset queuedSpellTimer 0
			} else {
				/echo QueueCast: Failed to cast ${spellName} due to ${castReturn}
			}
		} 
		/if (!${Bool[${Me.Gem[${spellName}]}]} && ${Me.Book[${spellName}]}) {
			/call memorize_spell "${spellName}" ${DefaultGem}
			/varset queuedSpellTimer 25
		}
		/if (!${Defined[castReturn]} || !${Bool[${castReturn}]} || !${castReturn.Equal[CAST_SUCCESS]}) {
			/if (${queuedSpellTimer.OriginalValue} >= 80) {
				/docommand ${ChatToggle} Failed to cast ${spellName} after retries
				/call PopStrQueue mySpellQueue
				/varset queuedSpellTimer 0
			} else /if (${Bool[${Me.Gem[${spellName}]}]} && ${Bool[${Me.GemTimer[${spellName}]}]}) {
				| Set timer to the the cooldown time
				/varset queuedSpellTimer ${Math.Calc[${Me.GemTimer[${spellName}]}/100]}
				/if (${queuedSpellTimer} >= 80) {
                    /docommand ${ChatToggle} Too long reuse timer to cast ${spellName}
                    /call PopStrQueue mySpellQueue
                    /varset queuedSpellTimer 0
                }
			} else /if (${Bool[${Me.AltAbility[${spellName}]}]} && ${Bool[${Me.AltAbilityTimer[${spellName}]}]}) {
                | Set timer to the the cooldown time
                /varset queuedSpellTimer ${Math.Calc[${Me.AltAbilityTimer[${spellName}]}/100]}
                /if (${queuedSpellTimer} >= 80) {
                    /docommand ${ChatToggle} Too long reuse timer to cast ${spellName}
                    /call PopStrQueue mySpellQueue
                    /varset queuedSpellTimer 0
                }
            } else /if (${queuedSpellTimer.OriginalValue} < 80 && ${queuedSpellTimer.OriginalValue} > 0) {
				/varset queuedSpellTimer ${Math.Calc[${queuedSpellTimer.OriginalValue}*2]}
			} else {
				/varset queuedSpellTimer 1s
			}
			/if (${queuedSpellTimer} > 0) {
				/echo QueueCast: will retry ${spellName} again in ${Math.Calc[${queuedSpellTimer}/10]} seconds
			}
		}
	}
/RETURN

| Calls e3_Cast for the given spell at the given target
SUB castQueuedSpell(spellName, targetID)
	/if (${DebugQueueCast}) /echo Casting queued ${spellName} on ${Spawn[id ${targetID}].Name} TargetID=${targetID}
	/declare whatToCast string local
	|Ewiclip -- Removed code that stopped spells with a ":" in the name from being castable.
	/if (${Bool[${Me.Book[${spellName}]}]} || ${Bool[${Me.AltAbility[${spellName}]}]}) {
		/if (${DebugQueueCast}) /echo Found spell or alt ability '${spellName}'
		/varset whatToCast ${spellName}
	} else /if (${Bool[${FindItemCount[=${spellName}]}]}) {
		/if (${DebugQueueCast}) /echo Found item '${spellName}'
		/varset whatToCast ${FindItem[=${spellName}]}
	}	

	/if (${DebugQueueCast}) /echo whatToCast = ${whatToCast}
	
	/if (${Bool[${whatToCast}]}) {
		/if (${Defined[queuecastArray]}) /deletevar queuecastArray
		/if (${Defined[queuecastArray2D]}) /deletevar queuecastArray2D		
		/declare queuecastArray[1] string outer ${whatToCast}
		/varset queuecastArray[1] ${whatToCast}
		/call BuildSpellArray "queuecastArray" "queuecastArray2D"
		/call e3_Cast ${targetID} "queuecastArray2D" 1
	}
/RETURN

| Add a spell to the queue with no targetid
#event queueCastNoTarget "<#1#> #2# queueCast #3#"
SUB event_queueCastNoTarget(line, ChatSender, Caster, spellName)
	/if (!${Me.Name.Equal[${Caster}]}) /return
	/if (${spellName.Find[targetid]}) {
		/return
	} 
	/if (!${Defined[targetID]}) /declare targetID int local 0
	/echo Queue casting ${spellName}

	/call AppendStrQueue mySpellQueue "${spellName},${targetID}"
/RETURN

| Adds spells to the queue with a targetid
#event queueCast "<#1#> #2# queueCast #3# targetid=#4#"
SUB event_queueCast(line, ChatSender, Caster, spellName, targetID)
	/if (!${Me.Name.Equal[${Caster}]}) /return
	/if (!${Defined[targetID]}) /declare targetID int local 0
	
	/echo Queue casting ${spellName} on ${Spawn[${targetID}].CleanName}

	/call AppendStrQueue mySpellQueue "${spellName},${targetID}"
/RETURN

|**
Queue implementation
This queue implementation is backed by a string
**|

| Used for debugging the queue methods
#event appendString "<#1#> appendString #2# #3#"
SUB event_appendString(line, ChatSender, variableName, newStr) 
	/call AppendStrQueue ${variableName} "${newStr}"
/RETURN

| Used for debugging the queue methods
#event popQueue "<#1#> popQueue #2#"
SUB event_popQueue(line, ChatSender, variableName) 
	/call PopStrQueue ${variableName}
	/echo Popped ${Macro.Return}
/RETURN

| Appends the given string to the variableName string
| will create variableName string if it doesn't exist
| You may need to pre-declare the variableName backing string
| in the Setup function to prevent issues with declaring outer variables
SUB StrAppend(variableName, newStr)
	/if (!${Defined[${variableName}]}) {
		/declare ${variableName} string outer ${newStr}
	} else {
		/varset ${variableName} ${${variableName}}${newStr}
	}
/RETURN

| Appends the given item to the queue (backed by a string value)
SUB AppendStrQueue(variableName, newItem)
	/if (!${Defined[queueSeparator]}) /declare queueSeparator string outer -
	/declare strWithSep string local ${newItem}${queueSeparator} 
	/call StrAppend ${variableName} "${strWithSep}"
/RETURN

| Peeks but does not remove the head item from the queue
| Get the result using ${Macro.Return}
SUB PeekStrQueue(variableName)
/RETURN ${${variableName}.Arg[1,${queueSeparator}]}

| Removes and retursn the head item from the queue
| Get the result using ${Macro.Return}
SUB PopStrQueue(variableName)
	/call PeekStrQueue ${variableName}
	/declare headValue string local ${Macro.Return}
	/declare positionOfSecondItem int local ${Math.Calc[${${variableName}.Find[${queueSeparator}]} + 1]}
	/declare totalLength int local ${${variableName}.Length}
	/declare remainingLength int local ${Math.Calc[${totalLength} - ${positionOfSecondItem} + 1]}
	/varset ${variableName} ${${variableName}.Mid[${positionOfSecondItem},${remainingLength}]}
/RETURN ${headValue}