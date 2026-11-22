// Design tokens shared with the dawbot plugin UI (dark surfaces, coral accent).

export const colors = {
  bg: '#161618',
  sheet: '#1D1D21',
  card: '#212125',
  raised: '#2A2A2F',
  line: 'rgba(255,255,255,0.07)',
  lineStrong: 'rgba(255,255,255,0.12)',
  text: '#F2F2F3',
  text2: '#A9A9B0',
  text3: '#85858B',
  coral: '#F4613F',
  coralText: '#FF8A70',
  coralBg: '#3A2520',
  coralMuted: '#D9B3A8',
  onCoral: '#141416',
  green: '#4CC38A',
  greenBg: '#1C2E25',
  red: '#FF7A7E',
  redBg: '#3A1F22',
  dim: 'rgba(8,8,10,0.62)',
  piano: '#F4F3F1',
  pianoLine: '#C9C7C2',
} as const;

export const fonts = {
  regular: 'Inter_400Regular',
  medium: 'Inter_500Medium',
  semibold: 'Inter_600SemiBold',
  brand: 'DMSans_600SemiBold',
} as const;

export const radius = { sm: 8, md: 12, lg: 14, xl: 16, xxl: 24, pill: 999 } as const;

export const space = { xs: 4, sm: 8, md: 12, lg: 16, xl: 20, xxl: 24, xxxl: 32 } as const;

export const type = {
  display: { fontFamily: fonts.semibold, fontSize: 30, lineHeight: 34, letterSpacing: -0.6, color: colors.text },
  title: { fontFamily: fonts.semibold, fontSize: 26, lineHeight: 30, letterSpacing: -0.5, color: colors.text },
  heading: { fontFamily: fonts.semibold, fontSize: 24, lineHeight: 30, color: colors.text },
  section: { fontFamily: fonts.semibold, fontSize: 17, lineHeight: 22, color: colors.text },
  body: { fontFamily: fonts.regular, fontSize: 15, lineHeight: 22, color: colors.text },
  bodyStrong: { fontFamily: fonts.semibold, fontSize: 15, lineHeight: 20, color: colors.text },
  sub: { fontFamily: fonts.regular, fontSize: 15, lineHeight: 22, color: colors.text2 },
  small: { fontFamily: fonts.regular, fontSize: 14, lineHeight: 20, color: colors.text2 },
  muted: { fontFamily: fonts.regular, fontSize: 13, lineHeight: 18, color: colors.text3 },
  eyebrow: { fontFamily: fonts.semibold, fontSize: 13, lineHeight: 18, color: colors.coralText },
} as const;
