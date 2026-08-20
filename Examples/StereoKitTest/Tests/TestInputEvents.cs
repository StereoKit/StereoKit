using StereoKit;
using System.Collections.Generic;

// Verifies the keyboard event queue itself, with no UI involved. Injections
// made during one frame are published at the start of the next, so each check
// runs one frame after the injections it describes.
class TestInputEvents : ITest
{
	int frame = 0;

	List<KeyboardEvent> ordered = new List<KeyboardEvent>();
	List<KeyboardEvent> repeats = new List<KeyboardEvent>();
	List<KeyboardEvent> mods    = new List<KeyboardEvent>();
	BtnState            repeatPolled;
	KeyboardEvent       astral;
	int                 indexedCount;
	KeyboardEvent       indexedFirst;
	KeyboardEvent       outOfRange;
	int                 afterIndexed;
	List<KeyboardEvent> filtered     = new List<KeyboardEvent>();
	List<uint>          legacyText   = new List<uint>();
	List<uint>          legacyReread = new List<uint>();
	uint                legacyAfterConsume;

	public void Initialize() => Tests.RunForFrames(10);

	public void Step()
	{
		switch (frame)
		{
			// Order: text and keys interleaved within a single frame must come
			// back out in exactly the order they went in.
			case 0:
				Input.TextInject      ("a");
				Input.KeyInjectPress  (Key.Backspace);
				Input.TextInject      ("b");
				Input.KeyInjectRelease(Key.Backspace);
				break;
			case 1:
				Drain(ordered);
				// Two presses with no release between them, as an auto-repeat
				// arrives. Both must survive as separate events.
				Input.KeyInjectPress(Key.Del);
				Input.KeyInjectPress(Key.Del);
				break;
			case 2:
				Drain(repeats);
				repeatPolled = Input.Key(Key.Del);
				Input.KeyInjectRelease(Key.Del);
				break;
			// Modifiers: each event carries the modifier state at the moment it
			// was produced, which polled state can't reproduce mid-frame.
			case 3:
				Input.KeyInjectPress  (Key.Shift);
				Input.TextInject      ("A");
				Input.KeyInjectRelease(Key.Shift);
				Input.TextInject      ("a");
				break;
			case 4:
				Drain(mods);
				Input.TextInject("\U0001F600");
				break;
			case 5:
				Input.KeyboardConsume(out astral);
				// Indexed reads see the whole frame, and consume nothing.
				indexedCount = Input.KeyboardEventCount;
				indexedFirst = Input.KeyboardEventAt(0);
				outOfRange   = Input.KeyboardEventAt(99);
				afterIndexed = Count(); // Consume had already emptied the queue
				Input.TextInject      ("xy");
				Input.KeyInjectPress  (Key.Home);
				break;
			// The deprecated cursor is independent: it sees only text events,
			// and reading through it doesn't disturb the event cursor.
			case 6:
#pragma warning disable CS0618
				for (char c = Input.TextConsume(); c != 0; c = Input.TextConsume())
					legacyText.Add(c);
				Input.TextReset();
				for (char c = Input.TextConsume(); c != 0; c = Input.TextConsume())
					legacyReread.Add(c);
				legacyAfterConsume = (uint)Count(); // Event cursor untouched
				// A lone surrogate half is not text, and must never reach a
				// reader, where KeyboardEvent.Text would throw on it.
				Input.TextInjectChar(0xD800);
#pragma warning restore CS0618
				// Carriage returns become newlines, and CRLF is a single one.
				Input.TextInject("a\r\nb\rc");
				Input.KeyInjectRelease(Key.Home);
				break;
			case 7:
				Drain(filtered);
				break;
			case 8:
				Tests.Test(OrderPreserved);
				Tests.Test(RepeatsAreSeparateEvents);
				Tests.Test(PolledKeyCollapsesRepeats);
				Tests.Test(ModifiersStampedPerEvent);
				Tests.Test(AstralCodepointSurvives);
				Tests.Test(IndexedReadsDontConsume);
				Tests.Test(CarriageReturnsBecomeNewlines);
				Tests.Test(LegacyCursorIsTextOnly);
				Tests.Test(LegacyCursorIsIndependent);
				break;
		}
		frame++;
	}

	static void Drain(List<KeyboardEvent> into)
	{
		while (Input.KeyboardConsume(out KeyboardEvent e)) into.Add(e);
	}

	static int Count()
	{
		int count = 0;
		while (Input.KeyboardConsume(out KeyboardEvent _)) count++;
		return count;
	}

	bool OrderPreserved()
		=>  ordered.Count == 4
		&&  ordered[0].type == KeyboardEventType.Text       && ordered[0].Text == "a"
		&&  ordered[1].type == KeyboardEventType.KeyPress   && ordered[1].key == Key.Backspace
		&&  ordered[2].type == KeyboardEventType.Text       && ordered[2].Text == "b"
		&&  ordered[3].type == KeyboardEventType.KeyRelease && ordered[3].key == Key.Backspace;

	// Two presses in one frame are two events. This is the fidelity the old
	// text-queue path had and polled keys never did.
	bool RepeatsAreSeparateEvents()
		=>  repeats.Count == 2
		&&  repeats[0].type == KeyboardEventType.KeyPress && repeats[0].key == Key.Del
		&&  repeats[1].type == KeyboardEventType.KeyPress && repeats[1].key == Key.Del;

	// The same two presses collapse to one JustActive, which documents why
	// editing keys read the queue rather than polling.
	bool PolledKeyCollapsesRepeats() => repeatPolled.IsJustActive();

	bool ModifiersStampedPerEvent()
		=>  mods.Count == 4
		// Shift's own press carries shift, its release does not.
		&&  (mods[0].modifiers & KeyMod.Shift) != 0
		&&  (mods[1].modifiers & KeyMod.Shift) != 0 && mods[1].Text == "A"
		&& !((mods[2].modifiers & KeyMod.Shift) != 0)
		&& !((mods[3].modifiers & KeyMod.Shift) != 0) && mods[3].Text == "a";

	bool AstralCodepointSurvives()
		=> astral.type == KeyboardEventType.Text && astral.Text == "\U0001F600";

	bool IndexedReadsDontConsume()
		=>  indexedCount == 1
		&&  indexedFirst.type == KeyboardEventType.Text && indexedFirst.Text == "\U0001F600"
		&&  outOfRange  .type == KeyboardEventType.None
		&&  afterIndexed == 0;

	// The surrogate injected alongside this text must not appear at all.
	bool CarriageReturnsBecomeNewlines()
	{
		List<KeyboardEvent> text = filtered.FindAll(e => e.type == KeyboardEventType.Text);
		return text.Count == 5
			&& text[0].Text == "a" && text[1].Text == "\n"
			&& text[2].Text == "b" && text[3].Text == "\n"
			&& text[4].Text == "c";
	}

	// Text events only, in order, and 'x','y' are the only text that frame.
	bool LegacyCursorIsTextOnly()
		=> legacyText.Count == 2 && legacyText[0] == 'x' && legacyText[1] == 'y';

	// TextReset re-reads the same text, and neither call moved the event cursor.
	bool LegacyCursorIsIndependent()
		=> legacyReread.Count == 2 && legacyReread[0] == 'x' && legacyAfterConsume == 3;

	public void Shutdown() { }
}
