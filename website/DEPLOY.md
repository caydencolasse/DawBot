# dawbot.ai landing page: deploy guide

This folder is a static site plus one serverless function. It's built for **Vercel**,
hosted from the `website/` folder of the `caydencolasse/DawBot` repo, and served on **dawbot.ai**.

```
website/
├── index.html          the landing page (desktop, tablet and mobile in one file)
├── api/waitlist.js     POST /api/waitlist → adds the email to your Brevo list
├── assets/             images, DAW icons, web fonts, social preview image
├── vercel.json         clean URLs, caching, www → apex redirect, security headers
├── favicon.png · apple-touch-icon.png · robots.txt · sitemap.xml
└── .env.example        the two environment variables the waitlist needs
```

## 1. Put it in the repo
From the repo root:
```bash
git checkout -b landing-page
# copy this "website" folder into the repo root, then:
git add website
git commit -m "Add dawbot.ai waitlist landing page"
git push -u origin landing-page
```
Merge the branch into `main`, either on GitHub or with `git checkout main && git merge landing-page && git push`.

## 2. Create the Brevo list (waitlist storage)
1. In Brevo, go to **Contacts → Lists → Create a list** and name it `dawbot waitlist`. Note its numeric **ID**.
2. In Brevo, go to **Contacts → Settings → Contact attributes** and add a **Text** attribute named `DAW`. This step is optional; without it, the page still saves emails.
3. In Brevo, go to **SMTP & API → API Keys → Generate a new API key** and copy it.

## 3. Deploy on Vercel
1. On vercel.com, go to **Add New → Project** and import `caydencolasse/DawBot`.
2. Set **Root Directory** to `website` and **Framework Preset** to **Other**. Leave the build command empty.
3. Under **Environment Variables**, add `BREVO_API_KEY` and `BREVO_LIST_ID`.
4. Click **Deploy**. Every push to `main` that touches `website/` redeploys automatically.

## 4. Point dawbot.ai at it
1. In the Vercel project, go to **Settings → Domains** and add `dawbot.ai` and `www.dawbot.ai`.
2. Vercel shows the DNS records to create. Add exactly those at the place you bought the domain.
   They're usually an **A** record for `@` and a **CNAME** for `www`.
3. Wait for both domains to show **Valid Configuration**. HTTPS is issued automatically.
   `www` redirects to `dawbot.ai` (see `vercel.json`).

## 5. Check it
- Open https://dawbot.ai on your phone and on a laptop.
- Join with a real email and confirm it appears in your Brevo list.
- Paste the URL into a Slack or iMessage chat to check the preview image.

## Fonts
- **DM Sans** and **Instrument Serif** load from Google Fonts.
- **ITC Avant Garde Gothic** is a paid font. The page uses it when it's installed, and otherwise falls back to
  TeX Gyre Adventor (a free, open-licensed look-alike in `assets/fonts/`).
  To use the real font on the web, buy a web license (Adobe Fonts or Monotype). Then add its
  `<link>` or `@font-face` and keep `"ITC Avant Garde Gothic Pro"` first in the font stack.

## Local preview
```bash
npm i -g vercel
cd website && cp .env.example .env.local   # fill in the two values
vercel dev                                  # http://localhost:3000
```
