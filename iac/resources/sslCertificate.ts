import { TerraformStack, TerraformVariable } from "cdktf";
import { AcmCertificate } from "@cdktf/provider-aws/lib/acm-certificate";
import { AcmCertificateValidation } from "@cdktf/provider-aws/lib/acm-certificate-validation";
import { DataAwsRoute53Zone } from "@cdktf/provider-aws/lib/data-aws-route53-zone";
import { Route53Record } from "@cdktf/provider-aws/lib/route53-record";

export function configureSSLCertificate(
  stack: TerraformStack,
  domainName: TerraformVariable,
  resourceTags?: any,
): AcmCertificateValidation {
  // create the certificate
  const certificate = new AcmCertificate(stack, "website-ssl-cert", {
    domainName: domainName.value,
    validationMethod: "DNS",
    lifecycle: {
      createBeforeDestroy: true,
    },
    tags: {
      ...resourceTags,
    },
  });

  // get the route53 zone
  const zone = new DataAwsRoute53Zone(stack, "ssl-route53-zone", {
    name: domainName.value,
    privateZone: false,
  });

  // create the route53 record
  const record = new Route53Record(stack, "ssl-route53-record", {
    dependsOn: [zone],
    name: certificate.domainValidationOptions.get(0).resourceRecordName,
    zoneId: zone.zoneId,
    type: certificate.domainValidationOptions.get(0).resourceRecordType,
    records: [certificate.domainValidationOptions.get(0).resourceRecordValue],
    ttl: 60,
  });

  // create the validation record
  const certificateValidation = new AcmCertificateValidation(
    stack,
    "website-ssl-certificate-validation",
    {
      certificateArn: certificate.arn,
      validationRecordFqdns: [record.fqdn],
    },
  );

  return certificateValidation;
}
