# Elliptangle

A TypeScript-based elliptangle renderer using HTML5 Canvas.

## Setup

1. Install dependencies:
```bash
npm install
```

2. Build the TypeScript code:
```bash
npm run build
```

3. Start the development server:
```bash
npm run serve
```

4. Open your browser to `http://localhost:8000`

## Development

- `npm run build` - Build TypeScript to JavaScript
- `npm run watch` - Watch for changes and rebuild automatically
- `npm run dev` - Watch for changes and serve files simultaneously
- `npm run serve` - Start a simple HTTP server

## Project Structure

```
├── src/
│   ├── elliptangle.ts    # Main elliptangle renderer class
│   └── main.ts           # Entry point that exports to global scope
├── dist/                 # Compiled JavaScript output
├── index.html            # Main HTML file
├── package.json          # NPM configuration
├── tsconfig.json         # TypeScript configuration
└── README.md            # This file
```

## Features

- Interactive controls for shape parameters
- Real-time rendering with curved borders
- Separate stroke colors for different sides
- TypeScript for type safety and better development experience
