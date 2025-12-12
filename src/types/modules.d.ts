declare module '@bytecodealliance/wizer' {
  const wizer: string;
  export default wizer;
}

declare module '@bytecodealliance/weval' {
  function getWeval(): Promise<string>;
  export default getWeval;
}
