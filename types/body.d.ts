declare module "fastly:body" {
  export class FastlyBody {
    constructor();
    concat(dest: FastlyBody): void;
    read(chunkSize: number): ArrayBuffer;
    append(data: BodyInit): void;
    prepend(data: BodyInit): void;
    close(): void;
  }
}
