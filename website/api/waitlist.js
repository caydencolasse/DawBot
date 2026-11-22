// Vercel serverless function: POST /api/waitlist
// Adds (or updates) a contact in your Brevo waitlist list.
//
// Environment variables (Vercel → Project → Settings → Environment Variables):
//   BREVO_API_KEY   your Brevo API key (Brevo → SMTP & API → API Keys)
//   BREVO_LIST_ID   the numeric id of your "dawbot waitlist" list (Brevo → Contacts → Lists)
//
// Optional: create a Brevo contact attribute named DAW (type: Text) to store
// the DAW people pick after joining.

const EMAIL_RE = /^[^@\s]+@[^@\s]+\.[a-z]{2,}$/i;
const DAWS = new Set([
  'Ableton Live', 'Logic Pro', 'FL Studio', 'Pro Tools',
  'Fender Studio Pro', 'GarageBand', 'Cubase', 'Other',
]);

async function addContact(email, attributes) {
  const body = {
    email,
    updateEnabled: true, // joining twice is fine: it just updates the contact
    listIds: [Number(process.env.BREVO_LIST_ID)],
  };
  if (attributes) body.attributes = attributes;

  return fetch('https://api.brevo.com/v3/contacts', {
    method: 'POST',
    headers: {
      'api-key': process.env.BREVO_API_KEY,
      'content-type': 'application/json',
      accept: 'application/json',
    },
    body: JSON.stringify(body),
  });
}

module.exports = async function handler(req, res) {
  if (req.method !== 'POST') {
    res.setHeader('Allow', 'POST');
    return res.status(405).json({ error: 'Method not allowed' });
  }

  if (!process.env.BREVO_API_KEY || !process.env.BREVO_LIST_ID) {
    console.error('waitlist: BREVO_API_KEY or BREVO_LIST_ID is not set');
    return res.status(500).json({ error: 'The waitlist is not set up yet. Please try again soon.' });
  }

  let payload = req.body || {};
  if (typeof payload === 'string') {
    try { payload = JSON.parse(payload); } catch { payload = {}; }
  }

  // Honeypot: real visitors never see or fill this field.
  if (payload.company) return res.status(200).json({ ok: true });

  const email = String(payload.email || '').replace(/\s+/g, '').toLowerCase();
  if (!EMAIL_RE.test(email) || email.length > 254) {
    return res.status(400).json({ error: 'That email looks incomplete. Check for a typo, like name@example.com.' });
  }

  const daw = DAWS.has(payload.daw) ? payload.daw : null;

  try {
    let r = await addContact(email, daw ? { DAW: daw } : undefined);

    // If the DAW attribute doesn't exist in Brevo yet, still save the email.
    if (!r.ok && daw) r = await addContact(email);

    if (r.ok) return res.status(200).json({ ok: true }); // 201 created or 204 updated

    console.error('waitlist: Brevo error', r.status, await r.text());
    return res.status(502).json({ error: "We couldn't add you right now. Please try again in a minute." });
  } catch (err) {
    console.error('waitlist: request failed', err);
    return res.status(502).json({ error: "We couldn't add you right now. Please try again in a minute." });
  }
};
