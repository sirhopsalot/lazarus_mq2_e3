|----------------------------------------------------------------------------|
| Returns these values: ${Cast.Result} and ${castReturn}
|- The vast majority of checks (distance/mana/hp/etc should be done by the funcion
|- that calls e3_casting (i.e e3_heals, e3_assist, e3_buffcheck)
|----------------------+----------------------------------------------------------------------|
| CAST_CANCELLED       | Spell was cancelled by ducking (either manually or because mob died) |
| CAST_CANNOTSEE       | You can't see your target                                            |
| CAST_IMMUNE          | Target is immune to this spell                                       |
| CAST_INTERRUPTED     | Casting was interrupted and exceeded the given time limit            |
| CAST_INVIS           | You were invis, and noInvis is set to true                           |
| CAST_NOTARGET        | You don't have a target selected for this spell                      |
| CAST_NOTMEMMED       | Spell is not memmed and you gem to mem was not specified             |
| CAST_NOTREADY        | AA ability or spell is not ready yet                                 |
| CAST_OUTOFMANA       | You don't have enough mana for this spell!                           |
| CAST_OUTOFRANGE      | Target is out of range                                               |
| CAST_RESIST          | Your spell was resisted!                                             |
| CAST_SUCCESS         | Your spell was cast successfully! (yay)                              |
| CAST_UNKNOWN         | Spell/Item/Ability was not found                                     |
| CAST_COLLAPSE        | Gate Collapsed                                                       |
| CAST_TAKEHOLD        | Spell not hold                                                       |
| CAST_FIZZLE          | Spell Fizzle                                                         |
| CAST_INVISIBLE       | NOT Casting Invis                                                    |
| CAST_RECOVER	       | Spell not Recovered yet!                                             |
| CAST_STUNNED	       | Stunned                                                              |
| CAST_STANDIG	       | Not Standing                                                         |
| CAST_DISTRACTED      | To Distracted ( spell book open )                                    |
| CAST_COMPONENTS| Missing Component													                                |
|---------------------------------------------------------------------------------------------|
Sub e3_Cast(int targetID, ArrayName, int ArrayIndex, bool StopEchos, bool NoDelay)
|StopEchos = Skips the /echo on successful cast if a value other than 0 is passed with the /call e3_Cast
/if (${Debug} || ${Debug_Casting}) /echo |- e3_Cast ==>

	

	/varset castReturn  
	/varset pendingCast	${${ArrayName}[${ArrayIndex},${iCastName}]}
	/varset pendingCastID ${${ArrayName}[${ArrayIndex},${iCastID}]}
	/varset castEndTime	0
	/varset castReturn 0
	/varset interruptFlag FALSE
	/declare pendingType string local
	/declare oldItem string local
	/declare i	int local
	/declare reEnableTwist bool local FALSE
	/declare e3_CastTime int local ${${ArrayName}[${ArrayIndex},${iMyCastTime}]}
	/declare afterSpell string local ${${ArrayName}[${ArrayIndex},${iAfterSpell}]}
	/declare castTargetName string local ${Spawn[id ${targetID}].CleanName}
	/declare e3_CastNoInterrupt bool local ${${ArrayName}[${ArrayIndex},${iNoInterrupt}]}
	/declare itemRecastToUseInSeconds int 60



	|this needs to be set to true every run, as its checked for interrupt method calls. Technically they set it to true every time they run, but best to 
	|be sure this is reset.
	/varset c_SubToRun TRUE
	|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	| Checks to abort casting due to previous failures
	|if im invis and iCastInvis=0 (default except for heals)
	|if im a rog, not in combat, done no action for 10+ seconds, and near 2+ other players then drop invis to buff (allows autohide)

	/if (${Me.Invis}) {
		|**/if (${Me.Class.ShortName.Equal[Rog]}) {
		/if (!${Me.Combat} && ${SpawnCount[pc radius 40]} > 2 && !${SpawnCount[npc radius 100 zradius 100]} && ${idleTimer}<${Math.Calc[${idleInterval}*600 - 100]}) /makemevisible
		} else**| 
		/if (!${${ArrayName}[${ArrayIndex},${iCastInvis}]}) {
				/varset castReturn CAST_INVISIBLE
				/echo [${Time}] SkipCast-Invis ${pendingCast} ${Spawn[id ${targetID}].Name} ${targetID}
				/varset ActionTaken TRUE
			/goto :skipCast
		}
	}
	|if we are a bard, and not doing async twisting this section of code
	|will 'block' as the check_BardSongs does not block on its call.
	|this allows other check_ Methods to run to do their computation while the spell is running
	|if they have to do an e3_Cast for their stuff, they will block here until the song is complete
	|if they don't do anything, it ends up blocking back in check_BardSongs for the next song on the list. 

	|START SPECIAL BARD CAST LOGIC
	|**
	A lot of casting logic is contained in this if statement
	The reason for this is to bypass all the logic for bards when casting an insta cast item while casting a song
	No other class can do two things at once, thus the special code. This is only done while not doing MQ2Twist
	**|
	/if (!${Twist.Twisting} && ${Me.Class.ShortName.Equal[BRD]}) {
	
		/if (${e3_CastTime}<500 ) {
			|can't play two items at once
			/if (!${bardPlayerCurrentSongType.Equal[item]} && ${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Item]}) {
				/if (${Defined[SpellRecast_${pendingCastID}]}) {
					
					/if (${SpellRecast_${pendingCastID}}) {
						|our timer is still active, skip this and say we are not recovered yet.
						|No action taken, so they can cast the next thing in the list.
						/echo [${Time}] ${pendingCast} was tried again before cooldown complete, skipping this action with a CAST_RECOVER return value
						/varset ActionTaken False
						/varset castReturn CAST_RECOVER
						/goto :skipCast
					}
				}
				/if (!${StopEchos}) /echo [${Time}]: ${pendingCast} ${pendingCastID} ${Spawn[id ${targetID}].Name} ${targetID}  (${Math.Calc[${e3_CastTime}/1000]}sec)
				/useitem ${pendingCast}
				/delay 3
				/call casting_ItemCooldown "${pendingCast}"
				/varset itemRecastToUseInSeconds ${Macro.Return}

				/if (${itemRecastToUseInSeconds}<60) {
				
					/varcalc itemRecastToUseInSeconds ${Math.Calc[${itemRecastToUseInSeconds}+1]}
				}
				/call CreateTimer "SpellRecast_${pendingCastID}" "${itemRecastToUseInSeconds}s"
				/varset castReturn CAST_SUCCESS
				/varset ActionTaken True
				/return
			} else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[AA]}) {
				/if (!${StopEchos}) /echo [${Time}]: ${pendingCast} ${pendingCastID} ${Spawn[id ${targetID}].Name} ${targetID}  (${Math.Calc[${e3_CastTime}/1000]}sec)
				/alt act ${Me.AltAbility[${pendingCast}].ID}
				/delay 10 !${Me.AbilityReady[${pendingCast}]}
				/varset castReturn CAST_SUCCESS
				/varset ActionTaken True
				
				/return
			}
		}
	

		/call BardSongWaitLockPumpQueue
	}
	|if we are not using MQ2Twist, but we currently have a cast going yet no cast window, issue a stopcast
	/if (!${Twist.Twisting} && ${Me.Class.ShortName.Equal[BRD]} && ${Bool[${Me.Casting.ID}]} && !${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Ability]} && !${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Disc]}) {
		|sometimes when the window is gone but the buff hasn't quite landed yet, stopcast can cancel it. delay 1 to make sure that situation is avoided
		/delay 1
		/stopcast
	}

	|END SPECIAL BARD CAST LOGIC

	|CatchAll Function for potentially locked up spell gems.
  	/call check_SpellGemLockup


  |/varset Debug_Casting TRUE
 
 

  |- Check if spell has a NOHOLD timer
  /if (${Defined[nht${targetID}-${pendingCastID}]}) {
    /if (${Bool[${nht${targetID}-${pendingCastID}}]}) {
      /varset castReturn CAST_TAKEHOLD
      /if (${Debug} || ${Debug_Casting}) /echo [${Time}] SkipCast-NoHold ${pendingCast} on ${Spawn[id ${targetID}].Name} Waiting [${nht${targetID}-${pendingCastID}}]
      /goto :skipCast
    } else {
      |/echo [${Time}] Deleting nht${targetID}-${pendingCastID} ${nht${targetID}-${pendingCastID}}
      /deletevar nht${targetID}-${pendingCastID}
    }
  }
	|- Check the Immune List
	/if (${Select[${pendingCastID}_${castTargetName},${immuneList}]} && !${pendingCast.Equal[${spammableSpell}]}) {
		/varset castReturn CAST_IMMUNE
    /if (${Debug} || ${Debug_Casting}) /echo [${Time}] SkipCast-Immune ${pendingCast} ${pendingCastID} ${Spawn[id ${targetID}].Name} ${targetID}
		/goto :skipCast 		
	}
	|- Check for spells listed in the failureList: reset on zone
	/if (${Select[${${ArrayName}[${ArrayIndex},${iCastName}]},${failureList}]}) {
		/varset castReturn FAILURELIST
    /if (${Debug} || ${Debug_Casting}) /echo [${Time}] SkipCast-Failure ${pendingCast} ${pendingCastID} ${Spawn[id ${targetID}].Name} ${targetID}
		/goto :skipCast 
	}
	|- Reagent Check *|
	/if (${Bool[${${ArrayName}[${ArrayIndex},${iReagent}]}]}) {
	  /declare itemCount int local ${FindItemCount[=${${ArrayName}[${ArrayIndex},${iReagent}]}]}
	  /if (!${itemCount}) {
      /bc Cannot cast [${pendingCast}], I do not have any [${${ArrayName}[${ArrayIndex},${iReagent}]}], removing this spell from array. Restock and Reload Macro
      /call RemoveSpellArrayElement "${ArrayName}" "${ArrayIndex}"
      /varset castReturn REAGENT
      /echo [${Time}] SkipCast-NoReagent ${pendingCast} ${pendingCastID} ${Spawn[id ${targetID}].Name} ${targetID}
      /beep
      /goto :skipCast
    } else /if (${itemCount} <= ${reagentWarningCount}) {
      /bc I only have ${itemCount} ${${ArrayName}[${ArrayIndex},${iReagent}]} left. Restock me!
    }
	}
  |- Dont cast while waiting for char to load post-zoning
  /if (${currentZone} != ${Zone.ID}) {
    /delay 1s
    /varset castReturn CAST_ZONING
    /goto :skipCast
  }

|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~		
|  Basic Checks
:wait_for_misc
  /if (${${ArrayName}[${ArrayIndex},${iTargetType}].Find[pet]}) {
    /if (!${Bool[${Me.Pet.ID}]} || ${Me.Pet.Distance} > ${${ArrayName}[${ArrayIndex},${iMyRange}]}) {
	  /call CreateTimer "nht${targetID}-${pendingCastID}" "${noHoldPetDelay}"
      /echo [${Time}] SkipCast-PetRange ${pendingCast} ${pendingCastID} ${Spawn[id ${targetID}].Name} ${targetID}
      /goto :skipCast
    }
  }
	/if (${Me.Feigning}) {
		/if (${Debug} || ${Debug_Casting}) /bc skipping [${pendingCast}] , i am feigned and noFeigning=${noFeigning}
		/goto :skipCast
	}
	/if (${Window[SpellBookWnd].Open}) {
    /if (${Debug} || ${Debug_Casting}) /bc skipping [${pendingCast}] , my spellbook is open
    /goto :skipCast
	}
	/if (${Corpse.Open}) {
		/if (${Debug} || ${Debug_Casting}) /bc skipping [${pendingCast}] , i have a corpse open and closeCorpse=${closeCorpse}
		/goto :skipCast
	}

  |if its not a beneficlal spell
  /if (!${${ArrayName}[${ArrayIndex},${iSpellType}].Find[Beneficial]}) {
     |if its not a self discipline
    /if (!(${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Disc]} && ${Spell[${${ArrayName}[${ArrayIndex},${iCastName}]}].TargetType.Equal[Self]})) {
		|if its not a PBAOE or Self buff
		/if (!${Select[${${ArrayName}[${ArrayIndex},${iTargetType}]},PB AE,Self]}) {
			|Do a Line of Sight Check!
			/if (!${Spawn[id ${targetID}].LineOfSight}) {
				/if (${Debug} || ${Debug_Casting}) /echo I cannot see [${Spawn[id ${targetID}].CleanName}]
				/varset castReturn CAST_CANNOTSEE
				/echo [${Time}] SkipCast-LoS ${pendingCast} ${pendingCastID} ${Spawn[id ${targetID}].Name} ${targetID}
				/goto :skipCast
			}
    	}
	}
  }


	|need to have a target so events/spells can have access to the target
	/if (!${Select[${${ArrayName}[${ArrayIndex},${iTargetType}]},PB AE,Self]}) {
		/call TrueTarget ${targetID}
	}
	/declare beforeEvent string local ${${ArrayName}[${ArrayIndex},${iBeforeEvent}]}
	/if (${beforeEvent.Length}>1) {
		/echo BeforeEvent configured, calling "${beforeEvent}" before ${pendingCast}
		/docommand ${beforeEvent}
	}
	/declare beforeSpell string local ${${ArrayName}[${ArrayIndex},${iBeforeSpell}]}
	/if (${beforeSpell.Length}>1) {
		/echo [${Time}] BeforeCast configured, calling "${beforeSpell}" before ${pendingCast}
		/call CastAfterBeforeSpell "${beforeSpell}"
		|reset global variables that will be cobbered by this call
		/varset castReturn  
		/varset pendingCast	${${ArrayName}[${ArrayIndex},${iCastName}]}
		/varset pendingCastID ${${ArrayName}[${ArrayIndex},${iCastID}]}
		/varset castEndTime	0
		/varset castReturn 0
		/varset interruptFlag FALSE
		
	}
	|if we are casting, lets clean up our cursor
	 /if (${Cursor.ID}) {	
		/echo [${Time}] <E3_Cast> Issuing auto inventory on ${Cursor} for spell: ${pendingCast}
		/autoinventory
	 }


	/declare startingLoc ${Me.Loc.Replace[ ,]}
|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|  Casting
:cast_spell
  |/if (${Me.Name.Equal[Lube]}) /gu Casting ${pendingCast} ${pendingCastID} ${Spawn[id ${targetID}].Name} ${targetID}
	/if (!${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Ability]}) {
		
		/if (!${StopEchos}) /echo [${Time}]: ${pendingCast} ${pendingCastID} ${Spawn[id ${targetID}].Name} ${targetID}  (${Math.Calc[${e3_CastTime}/1000]}sec)
	}
	|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	|Use Disc
	/if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Disc]}) {
		/if (${Me.ActiveDisc.ID} && ${Spell[${${ArrayName}[${ArrayIndex},${iCastName}]}].TargetType.Equal[Self]}) {
			/if (${Debug} || ${Debug_Casting}) /echo Skipping ${ArrayName}[${ArrayIndex},${iCastName}, Waiting on disc [${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text}]
			/varset castReturn ACTIVEDISC
			/goto :skipCast
		} else {
			| Only change targets if needed
			/if (${targetID} != ${Me.ID} && ${Target.ID} != ${targetID}) {
				/squelch /target id ${targetID}
				/delay 5 ${Target.ID} == ${targetID}
			}
			/varset ActionTaken TRUE
			/disc ${pendingCast}
			/delay 3
			/varset castReturn CAST_SUCCESS
            /varset ActionTaken True
		}
	|-~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	|- Abilities
	} else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Ability]}) {
		/if (${Me.AbilityReady[${pendingCast}]}) {
      	   /varset ActionTaken TRUE
		   |Slam is a special case and doesn't work with the normal /doability, this is a work around
		   /if (${pendingCast.Equal[Slam]}) {

				/if (${Window[ActionsAbilitiesPage].Child[AAP_FirstAbilityButton].Text.Equal[Slam]}) {
					/doability 1
				} else /if (${Window[ActionsAbilitiesPage].Child[AAP_SecondAbilityButton].Text.Equal[Slam]}) {
					/doability 2
				} else /if (${Window[ActionsAbilitiesPage].Child[AAP_ThirdAbilityButton].Text.Equal[Slam]}) {
					/doability 3
				} else /if (${Window[ActionsAbilitiesPage].Child[AAP_FourthAbilityButton].Text.Equal[Slam]}) {
					/doability 4
				} else /if (${Window[ActionsAbilitiesPage].Child[AAP_FifthAbilityButton].Text.Equal[Slam]}) {
					/doability 5
				} else /if (${Window[ActionsAbilitiesPage].Child[AAP_SixthAbilityButton].Text.Equal[Slam]}) {
					/doability 6
				}

		   } else {
		
			   /doability "${pendingCast}"
		
		   }
		   /delay 3 !${Me.AbilityReady[${pendingCast}]}
    	}
	|-~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	|- Spells, AA, Items
	} else {

		|if bard and the spell/item you are casting is > 0.3 sec stop what we are doing
		/if (${Select[${Me.Class.ShortName},BRD]} && ${e3_CastTime} >500) {
			/if (${Twist.Twisting}) {
				|this sets returnTwist to true
				/varset	reEnableTwist TRUE
				/call pauseTwist
				/delay 3s !${Me.Casting.ID}
				/delay 3
			} 
		}
	
		|- Stop following for spell/item/aa with a cast time > 0 MyCastTime, unless im a bard
		/if (${${ArrayName}[${ArrayIndex},${iMyCastTime}]} >0 && ${Me.Class.ShortName.NotEqual[BRD]}) {
			/if (${Stick.Status.Equal[on]}) /squelch /stick pause
			/if (${NetAdvPath.Following} && ${Following}) /squelch /netfollow off
			/if (${MoveTo.Moving}) /moveto off
			/delay 3 !${Me.Moving}
		}
		|- Equip an item that must be equipped to cast and is not currently equipped.  must use base inventory slots 24 through 31
	    /if (${Bool[${${ArrayName}[${ArrayIndex},${iItemMustEquip}]}]}) {
			
			/if (${FindItem[=${pendingCast}].ItemSlot} >=23) {
        	/varset oldItem ${Me.Inventory[${${ArrayName}[${ArrayIndex},${iItemMustEquip}]}].Name}
        	/call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange" "${oldItem}/${${ArrayName}[${ArrayIndex},${iItemMustEquip}]}" 1
        	/call SwapItem "${pendingCast}" "${${ArrayName}[${ArrayIndex},${iItemMustEquip}]}"
      		}
		}
		|- Define item|alt|gem# parameter
		/if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[AA]}) {
			/varset pendingType alt
			
		} else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Item]}) {
			/varset pendingType item
			
		} else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Spell]}) {
			|- if Gem specified, use it, else use default
			/if (${${ArrayName}[${ArrayIndex},${iSpellGem}]}) {
				/varset pendingType ${${ArrayName}[${ArrayIndex},${iSpellGem}]}
			} else {
				/varset pendingType ${DefaultGem}
			}
		}
		|/echo AT pendingCastID ${pendingCastID} pendingType ${pendingType}" "-targetid|${targetID}"
		/varset ActionTaken TRUE
		|to help with item clicks locking up, to give the server time to issue the cooldown.	
   		/if (${pendingType.Equal[item]}) {
   			|When an item delay is applied to an entry, check to see if the delay is still active
			/if (${Defined[SpellRecast_${pendingCastID}]}) {
				
			/if (${SpellRecast_${pendingCastID}}) {
				|our timer is still active, skip this and say we are not recovered yet.
				|No action taken, so they can cast the next thing in the list.
				/echo ${pendingCast} was tried again before cooldown complete, skipping this action with a CAST_RECOVER return value
				/varset ActionTaken False
				/varset castReturn CAST_RECOVER
				/goto :skipCast
			}
			}
		}
		/if (${${ArrayName}[${ArrayIndex},${iTargetType}].Equal[Self]}) {
			
			|bards songs don't work with /casting, have to do it ourselves
			/if (${Bool[${Me.Book[${pendingCast}]}]} && ${Me.Class.ShortName.Equal[BRD]}) {
				/echo e3_Cast issuing a raw cast for :"${pendingCast}"
				/cast "${pendingCast}"
                /delay 5 ${Window[CastingWindow].Open}
                /if (!${Window[CastingWindow].Open}) {
					/echo e3_Cast issuing stopcast as cast window isn't open
			        /stopcast
                    /delay 1
                    /cast ${pendingCast}
                    /delay 3
                }

                /varset castReturn CAST_SUCCESS
                /varset ActionTaken True

			} else {
				/casting "${pendingCast}|${pendingType}"
			}
			/if (!${Bool[${NoDelay}]}) {

				/delay 3
			} else {

				/echo <E3Cast> Casting without a delay! for ${pendingCast}
			}
			/declare castWindowWaitCounter int local 0
			/if (${e3_CastTime}>500) {
				/while (!${Window[CastingWindow].Open}) {
					/if (${Cast.Result.Equal[CAST_FIZZLE]}) {
						/echo window not open yet, but we have a fizzle set..breaking
						/break
					}
					/delay 1
					/varcalc castWindowWaitCounter ${castWindowWaitCounter}+1
					|if its taking longer than 2 sec to open a window we have an issue exit
					/if (${castWindowWaitCounter} > 20) {
						/break
					}
				}
			}
			
		} else {
			
			|bards songs don't work with /casting, have to do it ourselves
			/if (${Bool[${Me.Book[${pendingCast}]}]} && ${Me.Class.ShortName.Equal[BRD]}) {
				/call TrueTarget ${targetID}
				/cast "${pendingCast}"

                /delay 10 ${Window[CastingWindow].Open}
                
                /if (!${Window[CastingWindow].Open}) {
                    /stopcast
                    /delay 1
                    /cast ${pendingCast}
                    /delay 3
                }

                /varset castReturn CAST_SUCCESS
                /varset ActionTaken True

			} else {
				/casting "${pendingCast}|${pendingType}" "-targetid|${targetID}"
			}
			
			|delay 3 is enough time to have result come back with fizzle
			/if (!${Bool[${NoDelay}]}) {

				/delay 3
			} else {

				/echo <E3Cast> Casting without a delay! for ${pendingCast}
			}
			/declare castWindowWaitCounter int local 0
			/if (${e3_CastTime}>500) {
				/while (!${Window[CastingWindow].Open}) {
					/if (${Cast.Result.Equal[CAST_FIZZLE]}) {
						/echo window not open yet, but we have a fizzle set..breaking
						/break
					}
					/delay 1
					/varcalc castWindowWaitCounter ${castWindowWaitCounter}+1
					|if its taking longer than 2 sec to open a window we have an issue exit
					/if (${castWindowWaitCounter} > 20) {
						/break
					}
				}
			}
			
		}
		|- Memorizing spell
		/if (${Cast.Status.Find[M]}) /delay 3s !${Cast.Status.Find[M]}

		|- Set expected cast end time in 1/10ths of a second
		/varcalc castEndTime ${Me.Casting.MyCastTime}/100
		|/echo checking if window is open
:cast_still_pending
|- check for reasons to cancel casting
		
		/if (${Window[CastingWindow].Open}) {
		 
		  |dont allow item swap/must equip interrupts to avoid crashes
			|/echo checking is still open
		   	/if (${Me.Class.ShortName.Equal[BRD]}) {
				 /call ActivateSelosKick
		   }
		 	
		|-Check LifeSupport if more than 1.5 sec left to cast end
			/if (${Defined[lifeSupport2D]}) {
					/if (${Me.PctHPs}<70 && ${castEndTime}>15) /call check_lifeSupport
			}
			/if (${oldItem.Length} || ${e3_CastNoInterrupt}) {
				/delay 1
				/goto :cast_still_pending
			}

			/if (!${interruptFlag}) {
			   |- Run SubToRun
			    |/varset Debug_Casting TRUE
				|allow nothing to interrupt this.
			
				/if (${Bool[${${ArrayName}[${ArrayIndex},${iSubToRun}]}]}) {
					
					|call interrupt check
					/call ${${ArrayName}[${ArrayIndex},${iSubToRun}]} ${ArrayName} ${ArrayIndex}

					/if (!${c_SubToRun}) {
						/varset c_SubToRun True
						/echo [${Time}]: E3_Cast SubToRun called interrupt for spell: ${pendingCast}
					 	/call interrupt
						/varset ActionTaken True
						/varset castReturn CAST_INTERRUPTED
						/goto :skipCast
					}
					  /delay 1
        		|-no SubToRun
				} else {
				  /delay 1
				}
				|this will make tank/important heals ignore everything but subToRun
       			 /if (${Select[${ArrayName},tankHeals2D,importantHeals2D,xtargetHeals2D]}) /goto :cast_still_pending

				/doevents nowCast
				|check to see if now Cast has decided to interrupt the casting.
				/if (!${g_CastNowSpellCasting} && (!${c_SubToRun})) {
							/varset c_SubToRun True
							/echo [${Time}]: E3_Cast nowCast called interrupt for ${pendingCast}
							/call interrupt
							/delay 3
							/varset castReturn CAST_INTERRUPTED
							/goto :skipCast
				}

				|- Other reasons to cancel
				/if (!${Assisting} && ${Following} && !${Me.Class.ShortName.Equal[BRD]}) {
				/if (${SpawnCount[=${FollowTarget}]} && ${Spawn[=${FollowTarget}].Distance} < ${LeashLength}) {
					/if ((${Spawn[=${FollowTarget}].Distance} > 20 || ${NetBots[${FollowTarget}].Moving})) {
					/if (${Debug} || ${Debug_Casting}) /echo following interrupt
					|/bc following interrupt ${Time}  ${NetBots[${FollowTarget}].Moving}  ${Spawn[${FollowTarget}].Moving} ${Spawn[${FollowTarget}].Distance}
					/call interrupt
					/varset castReturn CAST_INTERRUPTED
					/goto :skipCast
					}
				}
				}
				|- goose
				/if (${Me.State.Equal[DUCK]}) {
					
					/if (${Debug} || ${Debug_Casting}) /echo duck called interrupt
							/call interrupt
							/varset castReturn CAST_INTERRUPTED
							/goto :skipCast
				}
						|-Dead target and I'm not using a detrimental AE or self buff
				/if (!${Select[${${ArrayName}[${ArrayIndex},${iTargetType}]},PB AE,Targeted AE,Corpse,Self,Group v1]}) {
					/if ((!${Bool[${Target.ID}]} || ${Spawn[${Target.ID}].Dead})) {
						/if (${Debug} || ${Debug_Casting}) /echo target corpse called interrupt
						/call interrupt
						/varset castReturn CAST_INTERRUPTED
						/goto :skipCast
					}
				}
						|-Target is out of range
				/if (${targetID} != ${Me.ID} && !${Select[${${ArrayName}[${ArrayIndex},${iTargetType}]},PB AE,Self]}) {
					/if (${Target.Distance} > ${${ArrayName}[${ArrayIndex},${iMyRange}]}) {
						/if (${Debug} || ${Debug_Casting}) /echo target range called interrupt
						/call interrupt
						/varset castReturn CAST_INTERRUPTED
						/goto :skipCast
					}
				}
						|-Moved since casting began
				/if (${castEndTime}>5 && !${Me.Class.ShortName.Equal[BRD]}) {
					
					/if (${Math.Distance[${startingLoc}]}>=8) {
						/if (${Debug} || ${Debug_Casting}) /echo moved 8+ units after casting: called interrupt
						/call interrupt
						/varset castReturn CAST_INTERRUPTED
						/goto :skipCast
					}
				}
				
				|-check only select background events
						/doevents Follow
						/doevents Stop
						/doevents MoveHere
						/doevents BackOff
						/doevents clickit
						/doevents bark
						/call alerts_Background_Events
			}
			/goto :cast_still_pending
		}
		
  :interrupted
		|only check casting if its not a bard spell, as it uses raw /cast
		/if (!(${Bool[${Me.Book[${pendingCast}]}]} && ${Me.Class.ShortName.Equal[BRD]})) { 
			|At this point the cast window is closed, but we may need to delay to get the result. 
			|note, if we don't find the result in 2 seconds, soemthing has gone wrong
			|it will use the previous cast return instead. There is a bug here for item lockups
			|so we let this continue to loop back around and remove the item lock
			/delay 2s !${Cast.Status.Find[C]}
			/varset castReturn ${Cast.Result}
			|- Cast Result Processing
				/if (${castReturn.Equal[CAST_INTERRUPTED]} || ${interruptFlag}) {
				/varset castReturn CAST_INTERRUPTED
			} else /if (${castReturn.Equal[CAST_SUCCESS]}) {
				/varset lastSuccessfulCast ${pendingCast}
			} else /if (${castReturn.Equal[CAST_RESIST]}) {
				/if (${Debug_Casting} && ${Target.ID}) /docommand ${ChatToggle} ** ${Target.Name} resisted ${pendingCast} **
			} else /if (${castReturn.Equal[CAST_IMMUNE]}) {
			/docommand ${ChatToggle} ${Target.Name} is immune to ${pendingCast} **
				/varset immuneList ${immuneList}${pendingCastID}_${castTargetName},
			|dont create a nohold timer for detrimental AE spells
			} else /if (${castReturn.Equal[CAST_TAKEHOLD]} && ${Select[${ArrayName},TargetAE_Spells2D,PBAE_Spells2D]}==0) {
					/if (${Target.ID}==${targetID}) {
					/docommand ${ChatToggle} ** ${pendingCast} did not take hold on ${castTargetName} **
					/call CreateTimer "nht${targetID}-${pendingCastID}" "${noHoldDelay}"
				}
			} else /if (${Select[${castReturn},CAST_OUTDOORS,CAST_COMPONENTS,CAST_UNKNOWN]}) {
				/docommand ** ${ChatToggle} Adding to failureList due to Return Events CAST_OUTDOORS,CAST_COMPONENT,CAST_UNKNOWN
				/varset failureList ${failureList}${pendingCastID},		
			}
		}
  }
	/if (${Debug} || ${Debug_Casting})  /echo |- e3_Cast -| castReturn= ${Cast.Result} ${castReturn}


	|to help with item clicks locking up, to give the server time to issue the cooldown.	
	/if (${Select[${castReturn},CAST_SUCCESS]} && ${pendingType.Equal[item]}) {
	
			|sometimes cooldowns are not just properly registered or there is a delay
			|add small cooldown of default 2 seconds, unless we know the cooldown of the item 
			|and we just set it ourselves.
			/call casting_ItemCooldown "${pendingCast}"
			/varset itemRecastToUseInSeconds ${Macro.Return}

			/if (${itemRecastToUseInSeconds}<60) {
				|/varset itemRecastToUseInSeconds ${Math.Calc[${itemRecastToUseInSeconds}]}
			} else {
				/varcalc itemRecastToUseInSeconds ${Math.Calc[${itemRecastToUseInSeconds}+6]}
			}
			/call CreateTimer "SpellRecast_${pendingCastID}" "${itemRecastToUseInSeconds}s"
	}

	/if (${afterSpell.Length}>1) {
		/echo [${Time}] AfterSpell configured, calling "${afterSpell}" for ${pendingCast}
		/call CastAfterBeforeSpell "${afterSpell}"
	}
	/declare afterEvent string local ${${ArrayName}[${ArrayIndex},${iAfterEvent}]}
	/if (${afterEvent.Length}>1) {
		|give time for buffs to land, in case they want toi remove one.
		/delay 5
		/echo [${Time}] AfterEvent configured, calling "${afterEvent}" for ${pendingCast}
		/docommand ${afterEvent}
	}

	:skipCast
	/if (${Defined[${ArrayName}]} && ${oldItem.Length}) {
		/call SwapItem "${oldItem}" "${${ArrayName}[${ArrayIndex},${iItemMustEquip}]}"
	}
	/if (${Following} && !${Assisting}) /call AcquireFollow
  	/if (${Me.Class.ShortName.Equal[BRD]}) /varset resumeTwistDelay 5
 	|if the twist was stopped for this.
	/if (${returnTwist}) {
		/call unpauseTwist
		
	}	
  
	|/varset Debug_Casting FALSE
	|sometimes cast status gets locked up
	|lets kill the current cast.
	
	|/echo Exiting e3_Cast with result : ${castReturn}
/return


sub CastAfterBeforeSpell(spellName)
	
	/if (${spellName.Length}>1) {
		/call check_ReadySimple "${spellName}"
        /if (${c_Ready}) {
			|need to check if there is a cooldown on it
			/declare spellItemID int local ${FindItem[${spellName}].ID}
			/if (${Defined[SpellRecast_${spellItemID}]}) {
						
				/if (${SpellRecast_${spellItemID}}) {
					|our timer is still active, skip this and say we are not recovered yet.
					|No action taken, so they can cast the next thing in the list.
					|/echo CastAfterBeforeSpell SpellRecast is on cooldown still for: ${SpellRecast_${spellItemID}} seconds
					/return
				}
			} 
			/call castSimpleSpell "${spellName}" 0
	    }
		
	}
/return

|------------------------------------------------|
|- Check if the Spell/Item/AA is ready			    -|
|------------------------------------------------|
Sub check_Ready(ArrayName, int ArrayIndex)
  /if (${Debug} || ${Debug_Casting}) /echo |- check_Ready =>
  /varset c_Ready FALSE
  | Memorize the spell if it is not memorized

  |if we are a bard, we need to check if we are already casting 
  |if we are , let the song finish before we check on the ready state of their spells
  |else they will basically never be
  /if (!${Twist.Twisting} && ${Me.Class.ShortName.Equal[BRD]}) {
	/while (${Window[CastingWindow].Open}) {
		/delay 1
	}
  }
  /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Spell]}) {
 
    /if (!${Bool[${Me.Gem[${${ArrayName}[${ArrayIndex},${iCastName}]}]}]}) {
 		/call memorize_spell "${${ArrayName}[${ArrayIndex},${iCastName}]}" ${${ArrayName}[${ArrayIndex},${iSpellGem}]}
	}
	:recheckCooldown
	/if ((${Bool[${Me.Gem[Focused Hail of Arrows]}]}  &&  ${Bool[${Me.Gem[Hail of Arrows]}]}) && (${${ArrayName}[${ArrayIndex},${iCastName}].Equal[Focused Hail of Arrows]} || ${${ArrayName}[${ArrayIndex},${iCastName}].Equal[Hail of Arrows]})) {
		|spell hard coded as ${Me.SpellReady} returns true when one is on cooldown because of a timer of another.
		/varset c_Ready TRUE
		/if (!${Me.SpellReady[Focused Hail of Arrows]}) /varset c_Ready FALSE
		/if (!${Me.SpellReady[Hail of Arrows]}) /varset c_Ready FALSE
		/return ${c_Ready}

	}
	/if ((${Bool[${Me.Gem[Mana Flare]}]}  &&  ${Bool[${Me.Gem[Mana Recursion]}]}) && (${${ArrayName}[${ArrayIndex},${iCastName}].Equal[Mana Flare]} || ${${ArrayName}[${ArrayIndex},${iCastName}].Equal[Mana Recursion]})) {
		|spell hard coded as ${Me.SpellReady} returns true when one is on cooldown because of a timer of another.
		/varset c_Ready TRUE
		/if (!${Me.SpellReady[Mana Flare]}) /varset c_Ready FALSE
		/if (!${Me.SpellReady[Mana Recursion]}) /varset c_Ready FALSE
		/return ${c_Ready}
	}
	/if (${Me.SpellReady[${${ArrayName}[${ArrayIndex},${iCastName}]}]}) {
		|verify the spell we want to use is actually ready
    
  	    /varset c_Ready TRUE
		/return ${c_Ready}
    }
	/if (${Me.SpellInCooldown} && ${Bool[${Me.Gem[${${ArrayName}[${ArrayIndex},${iCastName}]}]}]}) {
		|we are here because our spells are on cooldown, 
		/delay 1
		/goto :recheckCooldown
	}

 
  } else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Item]}) {
	|Need to check if we already have this item on cooldown
	/declare itemID int local ${FindItem[=${${ArrayName}[${ArrayIndex},${iCastName}]}].ID}
	/if (${Defined[SpellRecast_${itemID}]}) {
				
		/if (${SpellRecast_${itemID}}) {
			|our timer is still active, skip this and say we are not recovered yet.
			|No action taken, so they can cast the next thing in the list.
			|/echo check_Ready SpellRecast is on cooldown still for: ${SpellRecast_${itemID}}
			/varset c_Ready FALSE
			/goto :endOfCheckReady

		}
	} 
	/if (${Me.ItemReady[${${ArrayName}[${ArrayIndex},${iCastName}]}]}) {
		/varset c_Ready TRUE
	} 
  
  } else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[AA]}) {
    /if (${Me.AltAbilityReady[${${ArrayName}[${ArrayIndex},${iCastName}]}]})  /varset c_Ready TRUE
  } else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Disc]}) {
  			|/echo spellid to check :${${ArrayName}[${ArrayIndex},${iSpellID}]}
		/if (${${ArrayName}[${ArrayIndex},${iSpellID}]}==8001) {
			|bug with thiefs eyes, mark it as always true
			/varset c_Ready true
		} else /if (${Me.CombatAbilityReady[${${ArrayName}[${ArrayIndex},${iCastName}]}]}) {
			/varset c_Ready TRUE
		} 
  } else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Ability]}) {
    /if (${Me.AbilityReady[${${ArrayName}[${ArrayIndex},${iCastName}]}]}) /varset c_Ready TRUE
  }


	:endOfCheckReady

  /if (${Debug} || ${Debug_Casting}) /echo |- check_Ready <= ${c_Ready}

/RETURN ${c_Ready}


Sub check_ReadySimple(spellName, gemToUse)
  /if (${Debug} || ${Debug_Casting}) /echo |- check_ReadySimple =>
  /varset c_Ready FALSE
  |if we are a bard, we need to check if we are already casting 
  |if we are , let the song finish before we check on the ready state of their spells
  |else they will basically never be
  /if (!${Twist.Twisting} && ${Me.Class.ShortName.Equal[BRD]}) {
	/while (${Window[CastingWindow].Open}) {
		/delay 1
	}
  }
  | Memorize the spell if it is not memorized
  
	/declare spellType string local 

	/if (${Bool[${Me.AltAbility[${spellName}].Spell}]}) {
		/varset spellType AA
	} else /if (${Me.Book[${spellName}]}) {
		/varset spellType Spell
	} else /if (${Me.CombatAbility[${spellName}]}) {
		/varset spellType Disc
	} else /if (${Me.Ability[${spellName}]}) {
		/varset spellType Ability			
	} else {
		/varset spellType Item
	}
  /if (${spellType.Equal[Spell]}) {
    /if (!${Bool[${Me.Gem[${spellName}]}]}) {
		/declare tempGemNum int local ${DefaultGem}
		/if (${Bool[${gemToUse}]}) {
			/varset tempGemNum ${gemToUse}
		}
		/call memorize_spell "${spellName}" ${tempGemNum}
	}
	:recheckCooldown
     /if (${Me.SpellReady[${spellName}]}) {
		  /varset c_Ready TRUE
		  /RETURN ${c_Ready}
	}
	/if (${Me.SpellInCooldown} && ${Bool[${Me.Gem[${spellName}]}]}) {
		|we are here because our spells are on cooldown, 
		/delay 1
		/goto :recheckCooldown
	}


  } else /if (${spellType.Equal[Item]}) {
	|Need to check if we already have this item on cooldown
	/declare itemID int local ${FindItem[=${spellName}].ID}
	/if (${Defined[SpellRecast_${itemID}]}) {
				
		/if (${SpellRecast_${itemID}}) {
			|our timer is still active, skip this and say we are not recovered yet.
			|No action taken, so they can cast the next thing in the list.
			|/echo check_Ready SpellRecast is on cooldown still for: ${SpellRecast_${itemID}}
			/varset c_Ready FALSE
			/goto :check_ReadySimple

		}
	} 
	/if (${Me.ItemReady[=${spellName}]}) {
		/varset c_Ready TRUE
	} 
  
  } else /if (${spellType.Equal[AA]}) {
    /if (${Me.AltAbilityReady[${spellName}]})  /varset c_Ready TRUE
  } else /if (${spellType.Equal[Disc]}) {
    /if (${Me.CombatAbilityReady[${spellName}]}) /varset c_Ready TRUE
  } else /if (${spellType.Equal[Ability]}) {
    /if (${Me.AbilityReady[${spellName}]}) /varset c_Ready TRUE
  }
   :check_ReadySimple

  /if (${Debug} || ${Debug_Casting}) /echo |- check_ReadySimple <= ${c_Ready}

/RETURN ${c_Ready}

Sub memorize_spell(spellName,gemNum)
  /if (${Debug} || ${Debug_Casting}) /echo |- memorize_spell =>
  	/memorize "${spellName}" ${gemNum}

	/echo [${Time}] Delaying up to 5 seconds in memorize_spell
	/delay 5s ${Bool[${Me.Gem[${spellName}]}]}
	
	/if (!${Bool[${Me.Gem[${spellName}]}]}) {
		/echo [${Time}] Error spell not memorized kicking out of memorize_spell for spell: ${spellName} in gem ${gemNum}
		/return
	}

	|only wait for beneficial,self spells
	|(${Me.Combat} || ${Me.CombatState.Equal[Combat]} ||  ${AssistTarget} >0)
	/declare timeTowait int local ${Math.Calc[35+(${Spell[${spellName}].RecastTime}/100)]}
	/declare waitForReady timer local ${timeTowait}
	|/echo wfr ${waitForReady} rc ${Spell[${spellName}].RecastTime} math ${Math.Calc[25+(${Spell[${spellName}].RecastTime}/100)]}
	/echo [${Time}] Delaying waiting for spell to memorize and to be ready in memorize_spell: ${spellName} in gem ${gemNum}
	:checkTimer
	
	/if (${waitForReady}) {
		/call check_HealCasting_DuringDetrimental
		/if (!${c_SubToRun}) {
			/return
		}
		/doevents Follow
		/doevents Stop
		/doevents MoveHere
		/doevents BackOff
		/doevents Assist
		/call alerts_Background_Events

		/if ((${Me.Combat} || ${Me.CombatState.Equal[Combat]} ||  ${AssistTarget} >0)) {
			|kick out as we are now in combat
			/echo [${Time}] ABORT: Combat mode: Delaying waiting for spell to memorize and to be ready in memorize_spell: ${spellName} in gem ${gemNum}
			/return
		}

		/if (!${Me.SpellReady[${spellName}]}) {
			/delay 1
			/goto :checkTimer
		}
	}
	
  /if (${Debug} || ${Debug_Casting}) /echo |- memorize_spell <=
/RETURN
|------------------------------------------------|
|- concept from KA                        	    -|
|- MQ needs a .recovered function               -|
|------------------------------------------------|
Sub check_GCD
  /declare passCheck bool local FALSE
  /if (${Me.SpellReady[${Me.Gem[1].Name}]} || ${Me.SpellReady[${Me.Gem[3].Name}]} || ${Me.SpellReady[${Me.Gem[5].Name}]} || ${Me.SpellReady[${Me.Gem[7].Name}]}) /varset passCheck TRUE
/RETURN ${passCheck}

|------------------------------------------------|
|- Check if the Target is in Zone and in Range  -|
|------------------------------------------------|
Sub check_Distance(int targetID, int myRange)
	/declare c_Dist bool local FALSE
	/if (${Bool[${Spawn[id ${targetID}].Type.NotEqual[Corpse]}]}) {
	  /if (${myRange}==0 || ${Spawn[id ${targetID}].Distance} <= ${myRange}) /varset c_Dist TRUE
  }
/RETURN ${c_Dist}

|------------------------------------------------|
|- Check various Mana-related metrics		        -|
|------------------------------------------------|
Sub check_Mana(ArrayName, int ArrayIndex)
	/declare hasMana bool local FALSE
	/if (${Me.CurrentMana} >= ${${ArrayName}[${ArrayIndex},${iMana}]}) {
		/if (${Me.PctMana} >= ${${ArrayName}[${ArrayIndex},${iMinMana}]}) {
			/if (${Me.PctMana} <= ${${ArrayName}[${ArrayIndex},${iMaxMana}]}) /varset hasMana TRUE
		}
	}
/RETURN ${hasMana}

|------------------------------------------------|
|- Interrupt a spell when appropriate		    -|
|------------------------------------------------|
Sub interrupt
    /varset interruptFlag TRUE
    /if (!${NetBots[${Me.Name}].Mounted}) {
        /interrupt
    } 
	|we don't leave this method unless the previous spell is stopped
	|because of horses we just wait till the window is done, to preserve logic
	|flow
	/while (${Window[CastingWindow].Open}) {
		/delay 1
	}
	

/return

|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#EVENT vetAA "[#1#] VetAA #2#"
#EVENT vetAA "<#1#> VetAA #2#"
#EVENT vetAA "#1# tells you, 'VetAA #2#'"
#EVENT vetAA "#1# tells the group, 'VetAA #2#'"
#EVENT vetAA "#1# tell your party, 'VetAA #2#'"
SUB EVENT_vetAA(line, ChatSender,vetAAtoUse)
  /if (${line.Find[,]}) /varset line ${line.Left[${Math.Calc[${line.Find[,]}-1]}]} ${line.Right[-${line.Find[,]}]}
  /if (!${checkEventArgs[${ChatSender},${line},UZ,${vetAAtoUse}]}) /return
  /varset vetAAtoUse ${c_eventArgData}

  /declare i int local
  /for i 1 to ${vetArray2D.Size[1]}
  /if (${vetArray2D[${i},${iCastName}].Equal[${vetAAtoUse}]}) {
    /call check_Ready "vetArray2D" ${i}
    /if (${c_Ready}) {
      /if (${i}!=${iStaunch} || (${Me.PctMana}>30 && ${Me.PctEndurance}>30)) {
        /call e3_Cast ${Me.ID} "vetArray2D" ${i}
      }
    } else /echo ${vetAAtoUse} is not ready..
  }
  /next i
/RETURN

|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SUB casting_MacroSettings
/if (${Debug}) /echo |- casting_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},Debug,Debug Casting (On/Off)"  Off
	/call WriteToIni "${genSettings_Ini},Casting,Default Spell Set" Default
	/call WriteToIni "${genSettings_Ini},Casting,Default Spell Gem" ${If[${Me.AltAbility[Mnemonic Retention]},9,8]}
/if (${Debug}) /echo <== casting_MacroSettings -|
/RETURN

|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sub casting_Setup
/call iniToVarV "${advSettings_Ini},Debug,Debug Casting (On/Off)" Debug_Casting bool outer
/if (${Debug} || ${Debug_Casting}) /echo |- casting_Setup ==>
	|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	| Configurable
	/declare reagentWarningCount	int outer 5
	/declare noHoldDelay			int outer 600
	/declare noHoldPetDelay   int outer 300
	/declare closeCorpse			string outer FALSE
	|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	
	/declare pendingCast				string outer
	/declare pendingCastID			int outer
	/declare lastSuccessfulCast	string outer
	/declare castReturn					string outer
	/declare castEndTime				timer outer
	/declare noInvis						bool outer TRUE
	/declare noFeigning					bool outer TRUE
	/declare interruptFlag			bool outer FALSE

	/declare failureList			string outer
	/declare immuneList				string outer

	/declare c_Ready bool outer FALSE
	/declare c_SubToRun bool outer FALSE

	/declare DefaultGem int outer 8
	/call iniToVarV "${genSettings_Ini},Casting,Default Spell Gem" DefaultGem int outer
	/call iniToVarV "${genSettings_Ini},Casting,Default Spell Set" Default_SpellSet string outer

	/declare vetArray[8] string outer
	/declare iLesson    int outer 1
	|/declare iThrone    int outer 2
	/declare iArmor     int outer 2
	/declare iIntensity int outer 3
	/declare iImfusion  int outer 4
	/declare iSteadfast int outer 5
	/declare iStaunch   int outer 6
	/declare iExpedient int outer 7
	/declare iJester    int outer 8
	/varset vetArray[${iLesson}]    "Lesson of the Devoted"
	|/varset vetArray[${iThrone}]    "Throne of Heroes"
	/varset vetArray[${iArmor}]     "Armor of Experience"
	/varset vetArray[${iIntensity}] "Intensity of the Resolute"
	/varset vetArray[${iImfusion}]  "Infusion of the Faithful"
	/varset vetArray[${iSteadfast}] "Steadfast Servant"
	/varset vetArray[${iStaunch}]   "Staunch Recovery"
	/varset vetArray[${iExpedient}] "Expedient Recovery"
	/varset vetArray[${iJester}]    "Chaotic Jester"
	/call BuildSpellArray "vetArray" "vetArray2D"

	/declare itemCooldownList[58,2] string local
	/varset itemCooldownList[1,1] Summoned: Large Modulation Shard
	/varset itemCooldownList[1,2] 300
	/varset itemCooldownList[2,1] Shroud of the Subjugated Kuaan
	/varset itemCooldownList[2,2] 3600
	/varset itemCooldownList[3,1] Orb of Shadows
	/varset itemCooldownList[3,2] 30
	/varset itemCooldownList[4,1] Graverobber's Icon
	/varset itemCooldownList[4,2] 600
	/varset itemCooldownList[5,1] Aged Shissar Apothic Staff
	/varset itemCooldownList[5,2] 0
	/varset itemCooldownList[6,1] Weighted Hammer of Conviction
	/varset itemCooldownList[6,2] 0
	/varset itemCooldownList[7,1] Molten Orb
	/varset itemCooldownList[7,2] 12
	/varset itemCooldownList[8,1] Innoruuk's Dark Blassing
	/varset itemCooldownList[8,2] 300
	/varset itemCooldownList[9,1] Blood Drinker's Coating
	/varset itemCooldownList[9,2] 600
	/varset itemCooldownList[10,1] Blightbringer's Tunic of the Grave
	/varset itemCooldownList[10,2] 300
	/varset itemCooldownList[11,1] Dagger of Death
	/varset itemCooldownList[11,2] 120
	/varset itemCooldownList[12,1] Duskbringer's Plate Chestguard of the Hateful
	/varset itemCooldownList[12,2] 300
	/varset itemCooldownList[13,1] Minion's Memento
	/varset itemCooldownList[13,2] 600
	/varset itemCooldownList[14,1] Blessed Spiritstaff of the Heyokah
	/varset itemCooldownList[14,2] 180
	/varset itemCooldownList[15,1] Mindreaver's Vest  of Coercion
	/varset itemCooldownList[15,2] 30
	/varset itemCooldownList[16,1] Staff of Eternal Eloquence
	/varset itemCooldownList[16,2] 90
	/varset itemCooldownList[17,1] Aged Hammer of the Dragonborn
	/varset itemCooldownList[17,2] 210
	/varset itemCooldownList[18,1] Azure Mind Crystal III
	/varset itemCooldownList[18,2] 180
	/varset itemCooldownList[19,1] Zun'Muram's Spear of Doom
	/varset itemCooldownList[19,2] 210
	/varset itemCooldownList[20,1] Ritualchanter's Tunic of the Ancestors
	/varset itemCooldownList[20,2] 300
	/varset itemCooldownList[21,1] Hammer of Delusions
	/varset itemCooldownList[21,2] 120
	/varset itemCooldownList[22,1] Bladewhisper Chain Vest of Journeys
	/varset itemCooldownList[22,2] 300
	/varset itemCooldownList[23,1] Aurora, the Heartwood Bow
	/varset itemCooldownList[23,2] 180
	/varset itemCooldownList[24,1] Faithbringer's Breastplate of Conviction
	/varset itemCooldownList[24,2] 300
	/varset itemCooldownList[25,1] Aegis of Superior Divinity
	/varset itemCooldownList[25,2] 120
	/varset itemCooldownList[26,1] Glyphwielder's Tunic of the Summoner
	/varset itemCooldownList[26,2] 300
	/varset itemCooldownList[27,1] Vengeful Taelosian Blood Axe
	/varset itemCooldownList[27,2] 180
	/varset itemCooldownList[28,1] Focus of Primal Elements
	/varset itemCooldownList[28,2] 180
	/varset itemCooldownList[29,1] Wrathbringer's Chain Chestguard of the Vindicator
	/varset itemCooldownList[29,2] 300
	/varset itemCooldownList[30,1] Dagger of Evil Summons
	/varset itemCooldownList[30,2] 120
	/varset itemCooldownList[31,1] Aged Sarnak Channeler Staff
	/varset itemCooldownList[31,2] 120
	/varset itemCooldownList[32,1] Blade of Vesagran
	/varset itemCooldownList[32,2] 180
	/varset itemCooldownList[33,1] Kept-Covered Hammer
	/varset itemCooldownList[33,2] 210
	/varset itemCooldownList[34,1] Farseeker's Plate Chestguard of Harmony
	/varset itemCooldownList[34,2] 300
	/varset itemCooldownList[35,1] Everspring Jerkin of the Tangled Briars
	/varset itemCooldownList[35,2] 300
	/varset itemCooldownList[36,1] Master Lazarus Charm
	/varset itemCooldownList[36,2] 450
	/varset itemCooldownList[37,1] Staff of Everliving Brambles
	/varset itemCooldownList[37,2] 180
	/varset itemCooldownList[38,1] Gladiator's Plate Chestguard of War
	/varset itemCooldownList[38,2] 300
	/varset itemCooldownList[39,1] Kreljnok's Sword of Eternal Power
	/varset itemCooldownList[39,2] 60
	/varset itemCooldownList[40,1] Blood Drinker's Coating
	/varset itemCooldownList[40,2] 600
	/varset itemCooldownList[41,1] Bottomless Venom Vial
	/varset itemCooldownList[41,2] 180
	/varset itemCooldownList[42,1] Grobb Frenzy Maxx
	/varset itemCooldownList[42,2] 120
	/varset itemCooldownList[43,1] Fabled Idol of the Underking
	/varset itemCooldownList[43,2] 22
	/varset itemCooldownList[44,1] Fabled Idol of the UnderkingShattered Gnoll Slayer
	/varset itemCooldownList[44,2] 600
	/varset itemCooldownList[45,1] Rapier of Somber Notes
	/varset itemCooldownList[45,2] 120
	/varset itemCooldownList[46,1] Innoruuk's Voice
	/varset itemCooldownList[46,2] 480
	/varset itemCooldownList[47,1] Prismatic Dragon Blade
	/varset itemCooldownList[47,2] 300
	/varset itemCooldownList[48,1] Staff of Phenomenal Power
	/varset itemCooldownList[48,2] 180
	/varset itemCooldownList[49,1] Academic's Robe of the Arcanists
	/varset itemCooldownList[49,2] 300
	/varset itemCooldownList[50,1] Whispering Tunic of Shadows
	/varset itemCooldownList[50,2] 300
	/varset itemCooldownList[51,1] Nightshade, Blade of Entropy
	/varset itemCooldownList[51,2] 180
	/varset itemCooldownList[52,1] Dawnseeker's Chestpiece of the Defender
	/varset itemCooldownList[52,2] 300
	/varset itemCooldownList[53,1] Nightbane, Sword of the Valiant
	/varset itemCooldownList[53,2] 300
	/varset itemCooldownList[54,1] Deathwhisper
	/varset itemCooldownList[54,2] 180
	/varset itemCooldownList[55,1] Bifold Focus of the Evil Eye
	/varset itemCooldownList[55,2] 600
	/varset itemCooldownList[56,1] Transcended Fistwraps of Immortality
	/varset itemCooldownList[56,2] 180
	/varset itemCooldownList[57,1] Fiercehand Shroud of the Focused
	/varset itemCooldownList[57,2] 180
	/varset itemCooldownList[58,1] Invocation Rune: Vulka's Chant of Lightning
	/varset itemCooldownList[58,2] 18
  	|scan through each of the items in itemCooldownList to see if we actually have that item, if so put it in the lookup list.

	/declare tempItemLookupList[57,2] string local
	/declare tempItemLookupListCounter int local 1


	/declare index int local 1
	/for index 1 to ${itemCooldownList.Size[1]} 
		/if (${Bool[${FindItem[${itemCooldownList[${index},1]}]}]}) {
			|found the item, add the name/index in the primary list to the temp list.
			/varset tempItemLookupList[${tempItemLookupListCounter},1]  ${itemCooldownList[${index},1]}
			/varset tempItemLookupList[${tempItemLookupListCounter},2]  ${itemCooldownList[${index},2]}
			/varcalc tempItemLookupListCounter ${tempItemLookupListCounter}+1
		}
	/next index
	|creating our final list of only the size we need
	/declare itemCooldownLookupList[${Math.Calc[${tempItemLookupListCounter}-1]},2] string outer
	
	/if (${itemCooldownLookupList.Size[1]}>1) {

		/for index 1 to ${itemCooldownLookupList.Size[1]} 
				|found the item, add the name/index in the primary list to the temp list.
				/varset itemCooldownLookupList[${index},1]  ${tempItemLookupList[${index},1]}
				/varset itemCooldownLookupList[${index},2]  ${tempItemLookupList[${index},2]}
		/next index

		/echo Items to keep timers on.
		/for index 1 to ${itemCooldownLookupList.Size[1]}

			/echo ${index} ) ${itemCooldownLookupList[${index},1]}

		/next index

	}
	

/if (${Debug} || ${Debug_Casting}) /echo <== casting_Setup -|
/return


sub casting_ItemCooldown(itemName)

	/declare returnValue int local 2
	/declare i int local 0
	/for i 1 to ${itemCooldownLookupList.Size[1]}

			/if (${itemCooldownLookupList[${i},1].Equal[${itemName}]}) {
				/varset returnValue ${itemCooldownLookupList[${i},2]}
				/break
			}
	/next i
/return ${returnValue}

|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sub casting_Background_Events
  /doevents vetAA
/return

SUB casting_CharacterSettings
/return

Sub casting_Aliases
/return

SUB check_SpellGemLockup
	/declare UnlockSpell string local Origin
	/declare checkCounter int local 0

	/if (!${Me.AltAbilityReady[${UnlockSpell}]}) {
		/varset UnlockSpell "Marr's Calling"
	}
	
	|if we are stuck in this sitaution, delay for 0.3 sec to see if we will self correct before we do
	|anything drastic
	/while ((${Bool[${Me.Casting.ID}]} && !${Window[CastingWindow].Open} && !${Me.SpellInCooldown}) && ${checkCounter} < 3) {
		/delay 1
		/varcalc checkCounter ${checkCounter}+1 
	}

	/if (${Bool[${Me.Casting.ID}]} && !${Window[CastingWindow].Open} && !${Me.SpellInCooldown}) {
		/if (${Me.AltAbilityReady[${UnlockSpell}]}) {
			/bc ${Me.CleanName}'s spell gems are locked! Attempting unlock.
			/alt act ${Me.AltAbility[${UnlockSpell}].ID}

			/while (${Me.Casting.ID}) {
				/if (${Me.Mount.ID}) {
					/dismount
				}
				/interrupt
				/stopcast
				/delay 1
			}
		} else {
			/bc ${Me.CleanName}'s spell gems are locked! ${UnlockSpell} is not up!
		}
	}
/return
|--------------------------------------------------------------------------------------------|
|- Used to call the e3_Cast for simple spells that cannot normally fail.                   	-|
|--------------------------------------------------------------------------------------------|
sub castSimpleSpell(spellNameToBase, simpleTargetIDToBase, csNoDelay)

	
    /call castSimpleSpellBase "${spellNameToBase}" ${simpleTargetIDToBase} True ${Bool[${csNoDelay}]}

/return
sub castSimpleSpellBase(spellName, simpleTargetID, SkipParams, csbNoDelay)


	/if (${simpleTargetID}==0) {
		|if 0 passed in, set it to whatever they are targeting
		/varset simpleTargetID ${Target.ID}
	}
   	
	/call BuildSingleSpellArrayBase "${spellName}" "castSimpleSpellArray2D" ${SkipParams}
  
	/call check_Ready "castSimpleSpellArray2D" 1
	/declare checkReadyCounter int local 0
    /while (!${c_Ready}) {
		
		/delay 3
		/call check_Ready "castSimpleSpellArray2D" 1		
		/varcalc checkReadyCounter ${checkReadyCounter} +1
		/if (${checkReadyCounter} > 6) {
			/return
		}
	}
	
    /call e3_Cast ${simpleTargetID} "castSimpleSpellArray2D" 1 False  ${Bool[${csbNoDelay}]}
    /while (${Select[${castReturn},CAST_FIZZLE]}) {
          /delay 1s
          /call e3_Cast  ${simpleTargetID} "castSimpleSpellArray2D" 1 False ${Bool[${csbNoDelay}]}
    }

/return