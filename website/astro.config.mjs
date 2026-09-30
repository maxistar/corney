import { defineConfig } from 'astro/config';
import react from '@astrojs/react';

export default defineConfig({
  site: 'https://projects.maxistar.me',
  base: '/corney',
  output: 'static',
  integrations: [react()],
});
