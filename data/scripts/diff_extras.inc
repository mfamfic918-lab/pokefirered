@ Scripts for the features added by the difficulty builds (see include/difficulty.h).
@ The NPCs that use them are hidden in the original game (FLAG_DIFF_HIDE_EXTRA_NPCS).

	.if DIFFICULTY != 0

EventScript_DiffNuzlockeFallen::
	lockall
	buffernumberstring STR_VAR_1, VAR_0x40F9
	msgbox DiffNuzlocke_Text_Fallen
	releaseall
	end

Common_EventScript_TMVendor::
	lock
	faceplayer
	msgbox DiffExtras_Text_TMVendorGreeting
	callnative ShowRandomTMMart
	waitstate
	msgbox DiffExtras_Text_TMVendorBye
	release
	end

@ An NPC that trades a TM of the player's choice for Leftovers
Common_EventScript_LeftoversNPC::
	lock
	faceplayer
	msgbox DiffExtras_Text_LeftoversOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Common_EventScript_LeftoversNPC_Declined
	checkitemspace ITEM_LEFTOVERS
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_LeftoversNPC_NoSpace
	callnative OpenTMCaseForNPC
	waitstate
	goto_if_eq VAR_ITEM_ID, ITEM_NONE, Common_EventScript_LeftoversNPC_Declined
	removeitem VAR_ITEM_ID
	giveitem ITEM_LEFTOVERS
	msgbox DiffExtras_Text_LeftoversThanks
	release
	end

Common_EventScript_LeftoversNPC_Declined::
	msgbox DiffExtras_Text_LeftoversDeclined
	release
	end

Common_EventScript_LeftoversNPC_NoSpace::
	msgbox DiffExtras_Text_LeftoversNoSpace
	release
	end

	.else
EventScript_DiffNuzlockeFallen::
	end
@ The original game has no use for these NPCs, which are hidden; the scripts only need to exist
Common_EventScript_TMVendor::
Common_EventScript_LeftoversNPC::
	end
	.endif
